#include "utils.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/utils.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <format>

#include <wx/filedlg.h>

#include "config/misc/injection.hpp"
#include "config/priv/io.hpp"
#include "log/branch.hpp"
#include "log/context.hpp"
#include "ui/dialogs/message.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/string.hpp"

using namespace config::priv;

parse::CPPDirective parse::cppDirective(
    std::istream& stream,
    logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("config::parse::cppDirective()", lBranch)};

    enum class Read {
        None,
        Directive,
        Define_Pre,
        Define_Key,
        Define_Value,
        Include_Pre,
        Include,
        Ifdef_Pre,
        Ifdef,
        Unknown,
    } read{Read::None};

    std::string buf;
    std::string valBuf;

    CPPDirective ret;

    const auto processDirective{[&] {
        if (buf == "define") {
            read = Read::Define_Pre;
        } else if (buf == "include") {
            read = Read::Include_Pre;
        } else if (buf == "ifdef") {
            read = Read::Ifdef_Pre;
        } else if (buf == "endif") {
            ret.type_ = CPPDirective::Type::Endif;
            // Ignore rest
            read = Read::Unknown;
        } else {
            read = Read::Unknown;
            logger.warn("Ignoring unknown cpp directive \"" + buf + '"');
        }

        // Clear for key contents.
        buf.clear();
    }};

    const auto finish{[&] {
        if (read == Read::Directive)
            // Make sure this is called if a newline finishes things before the
            // normal handler can get to it.
            processDirective();

        switch (read) {
            case Read::None:
            case Read::Directive:
                logger.warn("Incomplete directive \"" + buf + '"');
                break;
            case Read::Define_Pre:
                logger.warn("Empty #define directive.");
                break;
            case Read::Define_Key:
            case Read::Define_Value:
                utils::trimSurroundingWhitespace(valBuf);
                ret.type_ = CPPDirective::Type::Define;
                ret.buf1_ = std::move(buf);
                ret.buf2_ = std::move(valBuf);
                break;
            case Read::Include_Pre:
                logger.warn("Empty #include directive.");
                break;
            case Read::Include:
                logger.warn("Ignoring unterminated #include directive.");
                break;
            case Read::Ifdef_Pre:
                logger.warn("Empty #ifdef directive.");
                break;
            case Read::Ifdef:
                ret.type_ = CPPDirective::Type::Ifdef;
                ret.buf1_ = std::move(buf);
                break;
            case Read::Unknown:
                break;
        }
    }};

    // A '#' has already been grabbed from stream.
    while (stream.good()) {
        utils::CommentData commentData{
            .stream_=stream,
            .single_=true,
            .skipNewlines_=false,
            .skipSpaces_=false,
        };
        if (utils::extractComments(commentData)) {
            // This means there's nothing else on the line
            if (commentData.type_ == utils::CommentData::eType_Line) {
                finish();
                return ret;
            }
        }

        auto chr{stream.get()};
        if (not stream.good())
            return ret;

        if (chr == '\n') {
            finish();
            return ret;
        }

        switch (read) {
            case Read::None:
                if (not std::isspace(chr)) {
                    stream.unget();
                    read = Read::Directive;
                }

                break;
            case Read::Directive:
                if (std::isspace(chr)) {
                    processDirective();
                    break;
                }

                buf += static_cast<char>(chr);
                break;
            case Read::Define_Pre:
                if (not std::isspace(chr)) {
                    stream.unget();
                    read = Read::Define_Key;
                }

                break;
            case Read::Define_Key:
                if (std::isspace(chr)) {
                    std::string tmp(buf);
                    uint32 numTrimmed{};
                    utils::trimCppName(tmp, false, &numTrimmed);

                    if (tmp != buf) {
                        logger.warn(wxString::Format(R"(Invalid define key "%s" trimmed to "%s")", buf, tmp).utf8_string());
                        buf = std::move(tmp);
                    }

                    if (buf.empty()) {
                        logger.warn("Ignoring define with empty key.");
                        read = Read::Unknown;
                    } else {
                        read = Read::Define_Value;
                    }

                    break;
                }

                buf += static_cast<char>(chr);
                break;
            case Read::Define_Value:
                // Everything else gets added.
                valBuf += static_cast<char>(chr);
                break;
            case Read::Include_Pre:
                if (chr == '"')
                    read = Read::Include;

                break;
            case Read::Include:
                if (chr == '"') {
                    ret.type_ = CPPDirective::Type::Include;
                    ret.buf1_ = std::move(buf);

                    // And ignore the rest of the line.
                    read = Read::Unknown;
                    break;
                }

                buf += static_cast<char>(chr);
                break;
            case Read::Ifdef_Pre:
                if (not std::isspace(chr)) {
                    stream.unget();
                    read = Read::Ifdef;
                }

                break;
            case Read::Ifdef:
                if (std::isspace(chr)) {
                    // Will handle the end for ifdef.
                    finish();

                    // Then discard everything else
                    read = Read::Unknown;
                }

                buf += static_cast<char>(chr);
                break;
            case Read::Unknown:
                break;
        }
    }

    return ret;
}

void parse::tryAddInjection(Config& config, const std::string& include) {
    auto& logger{logging::Context::getGlobal().createLogger("config::tryAddInjection()")};

    std::string_view file{include};

    auto injectionPos{include.find(INJECTION_STR)};
    if (injectionPos != std::string::npos) {
        logger.verbose("Injection string found...");

        // Skip over "injection/"
        file = file.substr(injectionPos + INJECTION_STR.length() + 1);
    }

    logger.debug(wxString::Format("Injection file: %s", file).utf8_string());

    if (
            file.find("../") != std::string::npos or
            file.find("/..") != std::string::npos
       ) {
        pcui::showMessage(
            wxString::Format(
                _("Injection file \"%s\" has an invalid name and cannot be registered.") + '\n' +
                _("You may add a substitute after import."),
                file
            ),
            {.caption_=_("Unknown Injection Encountered")}
        );
        return;
    }

    auto filePath{paths::injectionDir() / file};
    std::error_code ec;
    if (not fs::exists(filePath, ec)) {
        const auto registerChoice{pcui::showMessage(
            wxString::Format(
                _("Injection file \"%s\" has not been registered.") + '\n' +
                _("Would you like to add the injection file now?"),
                file
            ),
            {
                .caption_=_("Unknown Injection Encountered"),
                .style_=wxYES_NO | wxYES_DEFAULT
            }
        )};
        if (wxYES != registerChoice) return;

        fs::create_directories(filePath.parent_path(), ec);
        if (ec) {
            pcui::showMessage(
                ec.message(),
                {.caption_=_("Injection file directory could not be created.")}
            );
            return;
        }

        while (not false) {
            wxFileDialog fileDialog{
                nullptr,
                wxString::Format(_("Choose injection file for \"%s\""), file),
                wxEmptyString,
                wxEmptyString,
                "C Header (*.h)|*.h",
                wxFD_OPEN | wxFD_FILE_MUST_EXIST
            };
            if (fileDialog.ShowModal() == wxID_CANCEL) return;

            auto fromStr{fileDialog.GetPath().utf8_string()};
            if (not files::copyOverwrite(fromStr, filePath, ec)) {
                auto choice{pcui::showMessage(
                    ec.message(),
                    {
                        .caption_=_("Injection file could not be added."),
                        .style_=wxOK | wxCANCEL | wxOK_DEFAULT
                    }
                )};

                if (choice == wxCANCEL) return;

                continue;
            }
            break;
        }
    }

    config.injections_.append(
        std::make_unique<Injection>(config, std::string(file))
    );

    logger.debug("Done");
}

