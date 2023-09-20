#include "initializer.h"
#include "appstate.h"

std::string Initializer::message;
uint8_t Initializer::progress;

Initializer::Initializer() : wxProgressDialog("ProffieConfig Setup", "Initializing...", 100, nullptr, wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME) {}

void Initializer::setup() {
    if (!AppState::isInitialized) {
        //popen()
    }
}
