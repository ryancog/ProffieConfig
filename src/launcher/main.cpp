/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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

#include <wx/app.h>
#include <wx/utils.h>
#include <wx/progdlg.h>
#include <wx/snglinst.h>
#include <wx/msgdlg.h>

#include <app/app.h>
#include <log/context.h>
#include <log/logger.h>
#include <pconf/pconf.h>
#include <utils/paths.h>
#include <utils/defer.h>
#include <ui/message.h>

#include "routines.h"
#include "update/update.h"
#include "update/pulldata.h"
#include "update/changelog.h"
#include "update/install.h"

class Launcher : public wxApp {
public:
    bool OnInit() override {
        App::init("ProffieConfig Launcher");
        auto& logger{Log::Context::getGlobal().createLogger("Launcher")};

        wxSingleInstanceChecker checker;
        if (checker.IsAnotherRunning()) {
            PCUI::showMessage("A Launcher is Already Running", App::getAppName());
            return false;
        }

        logger.info("Checking installation status...");
        auto currentExec{Paths::executable()};
        auto installedExec{Paths::executable(Paths::Executable::LAUNCHER)};
        std::error_code err;
        if (not std::filesystem::equivalent(currentExec, installedExec, err)) {
            Routine::platformInstall(*logger.binfo("Launcher not installed, running install sequence..."));
            return false;
        }
        logger.info("Launcher installed, continuing...");

        enum Action {
            LAUNCH,
            FIRST_INSTALL,
            UNINSTALL
        } action{LAUNCH};
        
        if (argc == 2 and argv[1] == "uninstall") {
            action = UNINSTALL;
            if (wxNO == PCUI::showMessage("Are you sure you want to uninstall ProffieConfig?", App::getAppName(), wxYES_NO | wxNO_DEFAULT)) {
                return false;
            }
        } else {
            if (not std::filesystem::exists(Paths::executable(Paths::Executable::MAIN))) {
                action = FIRST_INSTALL;
                logger.info("Main ProffieConfig binary missing, update/install routine required.");
                if (wxNO == PCUI::showMessage("ProffieConfig installation needs to run, continue?", App::getAppName(), wxYES_NO | wxYES_DEFAULT)) {
                    return false;
                }
            }
        }

        string statusStr{};
        switch (action) {
            case LAUNCH:
                statusStr = "Update Check";
                break;
            case FIRST_INSTALL:
                statusStr = "First Install";
                break;
            case UNINSTALL:
                statusStr = "Uninstall";
                break;
        }
        PCUI::ProgressDialog prog(
                "ProffieConfig Launcher | " + statusStr, 
                "Initializing...", 100, nullptr, wxPD_APP_MODAL | (action != UNINSTALL ? wxPD_CAN_ABORT : 0));
        // To avoid lockup when full
        prog.SetRange(101);

        Update::init();

        if (action == LAUNCH or action == FIRST_INSTALL) {
            auto pullSuccess{Update::pullData(&prog, *logger.binfo("Collecting version data..."))};
            if (not pullSuccess) {
                logger.info("Aborting update after failed version data collection...");
                if (action == FIRST_INSTALL) {
                    PCUI::showMessage("Failed pulling update data, please try again!", App::getAppName());
                    return false;
                }

                Routine::launch(*logger.binfo("Launching without update..."));
                return false;
            }

            auto data{Update::parseData(&prog, *logger.binfo("Parsing version data..."))};
            if (not data) {
                PCUI::showMessage("Failed to parse data!\nPlease report this error.", App::getAppName());
                wxLaunchDefaultApplication(Paths::logs().string());
                return false;
            }

            if (data->bundles.empty()) {
                logger.error("No valid bundles found!");
                PCUI::showMessage("No valid version bundles found!\nPlease report this error.", App::getAppName());
                wxLaunchDefaultApplication(Paths::logs().string());
                if (action == LAUNCH) Routine::launch(*logger.binfo("Launching in lieu of valid update data."));
                return false;
            }

            Update::Version currentVersion;
            currentVersion = Update::determineCurrentVersion(data.value(), &prog, *logger.binfo("Determining current version."));
            auto changelog{Update::generateChangelog(data.value(), currentVersion, *logger.binfo("Generating changelog..."))};
            prog.Hide();
            if (changelog.currentBundleVersion and changelog.latestBundleVersion == changelog.currentBundleVersion) {
                Routine::launch(*logger.binfo("Up to date, launching..."));
                return false;
            }

            if (
                    action == LAUNCH and
                    not Update::promptWithChangelog(data.value(), changelog, *logger.binfo("Prompting user for update..."))
               ) {
                Routine::launch(*logger.binfo("User declined update."));
                return false;
            }
            prog.Show();

            if (not Update::pullNewFiles(changelog, data.value(), &prog, *logger.binfo("Downloading new files..."))) {
                prog.Close();
                logger.info("Aborting update after failed download.");
                if (action == LAUNCH) Routine::launch(*logger.binfo("Launching..."));
                return false;
            }

            Update::installFiles(changelog, data.value(), &prog, *logger.binfo("Installing files..."));

            prog.Hide();

            if (action == FIRST_INSTALL) PCUI::showMessage("Installed", App::getAppName());
        } else if (action == UNINSTALL) {
            Log::Context::destroyGlobal();

            std::error_code err;
            prog.Update(30, "Removing binaries...");
            fs::remove_all(Paths::binaries(), err);
            prog.Update(40, "Removing libraries...");
            fs::remove_all(Paths::libraries(), err);
            prog.Update(50, "Removing components...");
            fs::remove_all(Paths::components(), err);
            prog.Update(60, "Removing resources...");
            fs::remove_all(Paths::resources(), err);

            if (wxYES == PCUI::showMessage("Purge user data? (configurations, saves, etc.)\nIf files are kept, they will be available if reinstalled.", App::getAppName(), wxYES_NO | wxNO_DEFAULT)) {
                prog.Update(70, "Purging user data...");
                fs::remove_all(Paths::data());
            }

            prog.Update(90, "Removing platform setup...");
            Routine::platformUninstall();

            prog.Update(95, "Purging logs...");
            fs::remove_all(Paths::logs(), err);

            prog.Update(101, "Uninstalled.");
            return false;
        }

        Routine::launch(*logger.binfo("Launcher routines done."));
        return false;
    }

    void OnUnhandledException() override {
        App::exceptionHandler();
    }
};

wxIMPLEMENT_APP(Launcher);

