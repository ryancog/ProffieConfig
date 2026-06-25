/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * launcher/main.cpp
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

#include <thread>

#include <wx/app.h>
#include <wx/utils.h>
#include <wx/progdlg.h>
#include <wx/snglinst.h>
#include <wx/msgdlg.h>

#include "app/app.hpp"
#include "app/critical_dialog.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"
#include "utils/paths.hpp"
#include "ui/dialogs/message.hpp"

#include "routines.hpp"
#include "update/update.hpp"
#include "update/pulldata.hpp"
#include "update/changelog.hpp"
#include "update/install.hpp"

/**
 * This stuff works well enough, but virtually all the code in the launcher
 * is ugly. It could do with a lot of work at some point.
 */
class Launcher : public wxApp {
public:
    enum class Action {
        Launch,
        First_Install,
        Uninstall
    } action_{Action::Launch};

    pcui::ProgressDialog *prog_;
    logging::Logger *logger_;

    bool OnInit() override {
        app::setName("ProffieConfig Launcher");
        app::provideUI(pcui::showMessage);

        if (auto ec{paths::init()}) {
            pcui::showMessage(wxString::Format(
                _("Could not setup paths: %s"),
                ec.message()
            ));
            return false;
        }

        if (not app::setupExclusion("ProffieConfig")) {
            return false;
        }

        if (not app::init()) {
            app::CriticalDialog dlg(_("Initialization Failed"));
            dlg.ShowModal();
            return false;
        }

        logger_ = &logging::Context::getGlobal().createLogger("Launcher");
        auto& logger{*logger_};

        logger.info("Checking installation status...");
        auto currentExec{paths::executable()};
        auto installedExec{paths::executable(paths::Executable::Launcher)};
        std::error_code err;
        if (not fs::equivalent(currentExec, installedExec, err)) {
            routine::platformInstall(*logger.binfo("Launcher not installed, running install sequence..."));
            return false;
        }
        logger.info("Launcher installed, continuing...");

        if (argc == 2 and argv[1] == "uninstall") {
            action_ = Action::Uninstall;
            auto choice{pcui::showMessage(
                _("Are you sure you want to uninstall ProffieConfig?"),
                {.style_=wxYES_NO | wxNO_DEFAULT}
            )};
            if (wxNO == choice) {
                return false;
            }
        } else {
            if (not fs::exists(paths::executable(paths::Executable::Main))) {
                action_ = Action::First_Install;
                logger.info("Main ProffieConfig binary missing, update/install routine required.");

                auto choice{pcui::showMessage(
                    _("ProffieConfig installation needs to run, continue?"),
                    {.style_=wxYES_NO | wxYES_DEFAULT}
                )};
                if (wxNO == choice) {
                    return false;
                }
            }
        }

        wxString statusStr{};
        switch (action_) {
            case Action::Launch:
                statusStr = "Update Check";
                break;
            case Action::First_Install:
                statusStr = "First Install";
                break;
            case Action::Uninstall:
                statusStr = "Uninstall";
                break;
        }

        prog_ = new pcui::ProgressDialog(
            nullptr,
            "ProffieConfig Launcher | " + statusStr,
            true,
            wxSize{400, -1}
        );

        // Push full out of range.
        // TODO: This really should be changed so that each download fills
        // a respective portion of the progress...
        // If there is to be a progress bar for the current download, it should
        // be in conjunction with the overall progress.
        prog_->range(101);

        std::thread{&Launcher::doTheStuff, this}.detach();

        return true;
    }

    void doTheStuff() const {
        auto& prog{*prog_};
        auto& logger{*logger_};

        Update::init();

        if (action_ == Action::Launch or action_ == Action::First_Install) {
            checkAndUpdate();
        } else if (action_ == Action::Uninstall) {
            uninstall();
        }
    }

    void checkAndUpdate() const {
        auto& prog{*prog_};
        auto& logger{*logger_};

        auto pullSuccess{Update::pullData(&prog, *logger.binfo("Collecting version data..."))};
        if (not pullSuccess) {
            logger.info("Aborting update after failed version data collection...");
            if (action_ == Action::First_Install) {
                prog.finish(true, _("Failed pulling update data, please try again!"));
                return;
            }

            routine::launch(*logger.binfo("Launching without update..."));
            return;
        }

        auto data{Update::parseData(&prog, *logger.binfo("Parsing version data..."))};
        if (not data) {
            prog.finish(false);
            wxLaunchDefaultApplication(paths::logDir().string());
            return;
        }

        if (data->bundles.empty()) {
            logger.error("No valid bundles found!");
            prog.finish(true, _("No valid version bundles found!\nPlease report this error."));
            wxLaunchDefaultApplication(paths::logDir().string());
            if (action_ == Action::Launch) routine::launch(*logger.binfo("Launching in lieu of valid update data."));
            return;
        }

        utils::Version currentVersion;
        currentVersion = Update::determineCurrentVersion(data.value(), &prog, *logger.binfo("Determining current version."));
        auto changelog{Update::generateChangelog(data.value(), currentVersion, *logger.binfo("Generating changelog..."))};
        logger.info("Current Version: " + changelog.currentBundleVersion.string());
        logger.info("Latest Version: " + changelog.latestBundleVersion.string());

        prog.hide();

        if (
                changelog.currentBundleVersion and
                changelog.latestBundleVersion.compare(changelog.currentBundleVersion) == 0
            ) {
            prog.finish(false);

            routine::launch(*logger.binfo("Up to date, launching..."));
            return;
        }

        if (
                action_ == Action::Launch and
                not Update::promptWithChangelog(data.value(), changelog, *logger.binfo("Prompting user for update..."))
           ) {
            routine::launch(*logger.binfo("User declined update."));
            return;
        }
        prog.show();

        if (not Update::pullNewFiles(changelog, data.value(), &prog, *logger.binfo("Downloading new files..."))) {
            prog.finish(false);

            logger.info("Aborting update after failed download.");

            if (action_ == Action::Launch) routine::launch(*logger.binfo("Launching..."));
            return;
        }

        Update::installFiles(changelog, data.value(), &prog, *logger.binfo("Installing files..."));

        prog.pulse();
        prog.finish(action_ == Action::First_Install, _("Installed"));

        routine::launch(*logger.binfo("Launcher routines done."));
    }

    void uninstall() const {
        logging::Context::destroyGlobal();
        auto& prog{*prog_};

        std::error_code err;
        prog.set(30, "Removing binaries...");
        fs::remove_all(paths::binaryDir(), err);
        prog.set(40, "Removing libraries...");
        fs::remove_all(paths::libraryDir(), err);
        prog.set(50, "Removing components...");
        fs::remove_all(paths::componentDir(), err);
        prog.set(60, "Removing resources...");
        fs::remove_all(paths::resourceDir(), err);

        auto choice{pcui::showMessage(
            _("Purge user data? (configurations, saves, etc.)") + '\n' +
            _("If files are kept, they will be available if reinstalled."), 
            {.style_=wxYES_NO | wxNO_DEFAULT}
        )};
        if (wxYES == choice) {
            prog.set(70, "Purging user data...");
            fs::remove_all(paths::dataDir());
        }

        prog.set(90, "Removing platform setup...");
        routine::platformUninstall();

        prog.set(95, "Purging logs...");
        fs::remove_all(paths::logDir(), err);

        prog.set(95, "Finalizing...");
#       ifdef __APPLE__
        const auto currentBundle{
            paths::executable().parent_path().parent_path().parent_path()
        };
        fs::remove_all(currentBundle);
#       elif defined (_WIN32)
        MoveFileExW(
            paths::executable().c_str(),
            nullptr,
            MOVEFILE_DELAY_UNTIL_REBOOT
        );
#       elif defined (__linux__)
        (void)remove(paths::executable().c_str());
#       endif

        prog.set(101, "Uninstalled.");
    }
};

// NOLINTNEXTLINE(misc-use-internal-linkage)
wxIMPLEMENT_APP(Launcher);

