#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP "127.0.0.1"

int main(void) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    
    char userName[BUFFER_SIZE];
    double userBudget = 50000.0; // Araba için daha gerçekçi bir bütçe
    
    do {
        printf("İsminizi girin: ");
        fgets(userName, BUFFER_SIZE, stdin);
    } while(userName[0] == '\n');
    
    userName[strcspn(userName, "\n")] = 0;
    printf("Merhaba %s, Bütçeniz: $%.2f\n\n", userName, userBudget);
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Soket oluşturma hatası\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
        printf("Geçersiz adres\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Bağlantı başarısız. Lütfen TCP sunucusunu kontrol edin.\n");
        return -1;
    }

    read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);

    while (1) {
        printf("Komut girin: ");
        fgets(message, BUFFER_SIZE, stdin);

        // BUY komutu için özel işlem
        if (strncmp(message, "BUY ", 4) == 0) {
            message[strcspn(message, "\n")] = 0;
            // Modelin etrafına tek tırnak ekleme
            char tempMessage[BUFFER_SIZE];
            char model[BUFFER_SIZE];
            sscanf(message + 4, "%[^\n]", model); // 4 karakteri (BUY ) atla ve modeli al
            snprintf(tempMessage, BUFFER_SIZE, "BUY '%s' %.2f", model, userBudget);
            strncpy(message, tempMessage, BUFFER_SIZE);
            strcat(message, "\n");
        }
        
        send(sock, message, strlen(message), 0);

        if (strncmp(message, "EXIT", 4) == 0) {
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        long bytesRead = read(sock, buffer, BUFFER_SIZE);
        
        if (bytesRead < 0) {
            perror("Soketten okuma hatası");
            break;
        } else if (bytesRead == 0) {
            printf("Sunucu bağlantıyı kapattı.\n");
            break;
        }
        
        if (bytesRead > 0) {
            // "Kalan bütçe:" ifadesini kontrol et
            char *remainingMoneyPos = strstr(buffer, "Kalan bütçe:");
            if (remainingMoneyPos) {
                char carInfo[BUFFER_SIZE];
                char *periodPos = strchr(buffer, '\n');

                if (periodPos != NULL) {
                    size_t responseLength = periodPos - buffer;
                    strncpy(carInfo, buffer, responseLength);
                    carInfo[responseLength] = '\0';
                    printf("%s\n", carInfo);
                }
                
                double remainingMoney;
                sscanf(remainingMoneyPos, "Kalan bütçe: $%lf", &remainingMoney);
                userBudget = remainingMoney;
                printf("Güncel bütçeniz: $%.2f\n\n", userBudget);
            } else {
                printf("%s\n", buffer);
            }
        }
    }

    close(sock);
    return 0;
}
