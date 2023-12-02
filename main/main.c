#include "webserver.h"
#include "board.h"

int app_main() {
    board_init();
    webserver_run();
    return 0;
}


