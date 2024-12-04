#include <windows.h>
#include <toml.h>
#include <ReadBarcode.h>
#include <stddef.h>
#include <stdint.h>

#include <mutex>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <condition_variable>

#define ms() (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())

namespace scanner {
    typedef bool (*CommitCardCallback) (std::string, std::string);
    typedef bool (*CommitQrCallback)   (std::vector<uint8_t> &);
    typedef void (*OnDataReceive)      (std::vector<uint8_t> &);

    namespace qrcode {
        void Init (HINSTANCE lib, OnDataReceive callback);
        void ChangeStatus (bool active);
        void Exit ();
    }
    void MainLoop ();
}