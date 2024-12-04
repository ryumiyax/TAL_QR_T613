#include "scanner.h"

namespace scanner::qrcode {
    bool connected = false, connectedFlip = false;
    bool active = false, activeFlip = false;

    OnDataReceive callback;

    typedef int32_t(*ptx_scanner_init)(void);                                                           // 初始化扫码器DLL资源
    typedef int32_t(*ptx_scanner_deinit)(void);                                                         // 释放扫码器资源
    typedef int32_t(*ts_scan_callback)(uint8_t ucCodeType, uint8_t* pBuf, uint32_t uiBufLen);           // 扫码信息的回调函数
    typedef void (*ts_state_callback)(uint8_t ucState);                                                 // 扫码器的连接回调
    typedef int32_t(*ptx_scanner_decode_data_fun_register)(ts_scan_callback fScanCallback);             // 注册 扫码信息的回调函数
    typedef int32_t(*ptx_scanner_comm_state_fun_register)(ts_state_callback fStateFun);                 // 注册 扫码器的连接回调
    typedef int32_t(*ptx_scanner_get_light_state)(void);                                                // 获取当前灯光状态
    typedef int32_t(*ptx_scanner_set_light_state)(uint8_t ucMode, uint8_t isTemp);                      // 灯光状态控制 1,0:常亮  2,0:关闭
    typedef int32_t(*ptx_scanner_scan_sw)(uint8_t ucEnable);                                            // 扫码状态控制 0:关闭扫码  1:开启扫码
    
    ptx_scanner_init tx_scanner_init;
    ptx_scanner_deinit tx_scanner_deinit;
    ptx_scanner_decode_data_fun_register tx_scanner_decode_data_fun_register;
    ptx_scanner_comm_state_fun_register tx_scanner_comm_state_fun_register;
    ptx_scanner_get_light_state tx_scanner_get_light_state;
    ptx_scanner_set_light_state tx_scanner_set_light_state;
    ptx_scanner_scan_sw tx_scanner_scan_sw;

    int32_t scan_callback(uint8_t codeType, uint8_t* pBuf, uint32_t uiBufLen) {
        if ((NULL == pBuf) || (0 == uiBufLen)) return -1;
        std::cout << "[QR][T613] data received len=" << uiBufLen << std::endl;
        std::vector<uint8_t> data = {};
        for (uint32_t i = 0; i < uiBufLen; i ++) {
            data.push_back(pBuf[i]);
        }
        if (active) callback (data);
        return 0;
    }

    void state_callback(uint8_t ucState) {
        std::cout << "[QR][T613] SCANNER " << ((ucState) ? "Connected!" : "Disconnected!") << std::endl;
        connectedFlip = connected ^ (bool)ucState;
        connected = (bool)ucState;
    }

    bool alive = true;
    std::mutex loopMutex;
    std::condition_variable loopCV;
    std::thread mainLoop;
    void MainLoop () {
        std::unique_lock<std::mutex> loopLock(loopMutex);
        while (alive) {
            if (connectedFlip && connected) {
                connectedFlip = false;
                bool light = tx_scanner_get_light_state ();     // 默认灯光配置
                if (light) tx_scanner_set_light_state (1, 1);   // 不为熄灭时写入默认熄灭(持久化，连接上即为熄灭状态)
                activeFlip = false;
                if (active) tx_scanner_set_light_state (2, 0);  // 但如果当前灯光状态应为点亮时初始化为点亮
                tx_scanner_scan_sw (active);                    // 同步修改扫码状态为当前生效状态
            } else if (connected) {
                if (activeFlip) {
                    activeFlip = false;
                    tx_scanner_scan_sw (active);
                    tx_scanner_set_light_state ((uint8_t)(active + 1), 0);
                }
            }
            if (alive) loopCV.wait_for (loopLock, std::chrono::milliseconds(200));
        }
        tx_scanner_scan_sw (active);
        tx_scanner_set_light_state ((uint8_t)(active + 1), 0);
        tx_scanner_deinit ();
    }

    void ChangeStatus (bool active) {
        activeFlip = scanner::qrcode::active ^ active;
        scanner::qrcode::active = active;
        loopCV.notify_all ();
    }

    void* initMethod(HINSTANCE lib, const char* method_name) {
        void* pMethod = (void*)GetProcAddress(lib, method_name);
        if (pMethod == NULL) std::cout << "[QR][T613] Init " << method_name << " Method Error: " << GetLastError() << std::endl;
        return pMethod;
    }

    void Init (HINSTANCE lib, OnDataReceive callback) {
        if (lib == nullptr) return;
        scanner::qrcode::callback = callback;
        tx_scanner_init = (ptx_scanner_init)initMethod (lib, "tx_scanner_init");
        tx_scanner_deinit = (ptx_scanner_deinit)initMethod (lib, "tx_scanner_deinit");
        tx_scanner_decode_data_fun_register = (ptx_scanner_decode_data_fun_register)initMethod (lib, "tx_scanner_decode_data_fun_register");
        tx_scanner_comm_state_fun_register = (ptx_scanner_comm_state_fun_register)initMethod (lib, "tx_scanner_comm_state_fun_register");
        tx_scanner_get_light_state = (ptx_scanner_get_light_state)initMethod (lib, "tx_scanner_get_light_state");
        tx_scanner_set_light_state = (ptx_scanner_set_light_state)initMethod (lib, "tx_scanner_set_light_state");
        tx_scanner_scan_sw = (ptx_scanner_scan_sw)initMethod (lib, "tx_scanner_scan_sw");

        tx_scanner_decode_data_fun_register (scan_callback);
        tx_scanner_comm_state_fun_register (state_callback);
        tx_scanner_init ();
        mainLoop = std::thread (MainLoop);
        mainLoop.detach ();
    }

    void Exit () {
        ChangeStatus (false);
        alive = false;
        loopCV.notify_all ();
    }

}