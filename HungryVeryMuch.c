#include "pideLib.h"

int main(int argc, char
    const * argv[]) {

    if (argc != 5) {
        printf("Usage: HungryVeryMuch [portnumber] [numberOfClients] [p] [q]\n");
        return 1;
    }

    const char * portnumber = argv[1];
    const char * numberOfClients_string = argv[2];
    const char * p_string = argv[3];
    const char * q_string = argv[4];

    int numberOfClients = atoi(numberOfClients_string);
    int p = atoi(p_string);
    int q = atoi(q_string);

    printf("portnumber: %s\nnumberOfClients: %d\np: %d\nq: %d\n\n", portnumber, numberOfClients, p, q);

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {
        0
    };

    int port = atoi(portnumber);

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", & serv_addr.sin_addr) <=
        0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr * ) & serv_addr,
            sizeof(serv_addr))) <
        0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    int networkOrder = htonl(numberOfClients);

    send(client_fd, & networkOrder, sizeof(networkOrder), 0);

    int networkp = htonl(p);

    send(client_fd, & networkp, sizeof(networkp), 0);

    int networkq = htonl(q);

    send(client_fd, & networkq, sizeof(networkq), 0);

    Order * order = (Order * ) malloc(sizeof(Order));

    srand(time(NULL));

    for (int i = 0; i < numberOfClients; i++) {

        order -> p = rand() % (p + 1);
        order -> q = rand() % (q + 1);
        order -> num = i + 1;

        printf("order->p: %d\n", order -> p);
        printf("order->q: %d\n", order -> q);

        send(client_fd, order, sizeof(Order), 0);
        printf("Order-%d sent\n\n", i + 1);
    }

    valread = read(client_fd, buffer,
        1024 - 1);

    printf("%s\n", buffer);

    close(client_fd);
    return 0;
}
