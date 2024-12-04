#include "scanner/scanner.h"

namespace scanner {
    CommitCardCallback cardCallback;
    CommitQrCallback qrCallback;
    bool cardStatus  = false;
    bool qrStatus    = false;
    bool loginByCard = true;
    bool alive       = true;

    std::mutex mtx;
    std::thread mainLoop;
    std::condition_variable cv;
    std::string accessCodeHead = "BNTTCNID";

    bool CheckAccessCode (std::vector<uint8_t> &buffer) {
        for (int i = 0; i < 8; i++) {
            if (buffer[i] != accessCodeHead[i]) return false;
        }
        return true;
    }

    void HandleData (std::vector<uint8_t> &buffer) {
        if (loginByCard && cardStatus && buffer.size () > 8 && CheckAccessCode (buffer)) {
            std::string accessCode = "";
            for (size_t i = 8; i < buffer.size (); i++) accessCode += buffer[i];
            bool accepted = cardCallback (accessCode, "");
            if (accepted) {
                buffer.clear ();
                return;
            }
        }
        qrCallback (buffer);
        buffer.clear ();
    }

    void MainLoop () {
        HINSTANCE t613Lib = LoadLibraryW(L"hidpos.dll");
        if (t613Lib != nullptr) {
            // std::cout << "[QR][T613] init with dll!" << std::endl;
            scanner::qrcode::Init (t613Lib, scanner::HandleData);
        } else {
            std::cout << "[QR][T613] Cannot find hidpos.dll, scanner will not connect!" << std::endl;
        }
    }
}

extern "C" {
    void InitCardReader (scanner::CommitCardCallback callback) {
        // std::cout << "[QR][T613] Got CardReaderCallback!" << std::endl;
        scanner::cardCallback = callback;
    }

    void InitQRScanner (scanner::CommitQrCallback callback) {
        // std::cout << "[QR][T613] Got QRScannerCallback!" << std::endl;
        scanner::qrCallback = callback;
    }

    void UpdateStatus (size_t type, bool status) {
        // std::cout << "[QR][T613] Got UpdateStatus type=" << type << " status=" << status << std::endl;
        if (type == 1) scanner::cardStatus = status;
        if (type == 2) scanner::qrStatus   = status;
        scanner::qrcode::ChangeStatus (scanner::qrStatus || scanner::cardStatus);
    }

    void InitVersion (uint64_t gameVersion) {
        if (gameVersion == 0xA7EE39F2CC2C57C8ull) {
            std::cout << "[QR][T613] CHN00 detected, disable card login simulation!" << std::endl;
            scanner::loginByCard = false;
        }
    }

    void Init () {
        // std::cout << "[QR][T613] Init" << std::endl;
        scanner::mainLoop = std::thread (scanner::MainLoop);
        scanner::mainLoop.detach ();
    }

    void Exit () {
        // std::cout << "[QR][T613] Exit!" << std::endl;
        scanner::qrcode::Exit ();
    }
}