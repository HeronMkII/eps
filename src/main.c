#include <uart/uart.h>
#include <uart/log.h>

int main() {
    init_uart();
    print("Hello EPS!\n");
}
