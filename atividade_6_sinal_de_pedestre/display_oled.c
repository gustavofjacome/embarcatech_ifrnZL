#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define BTN_A_PIN 5

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

int A_state = 0; // Botão A está pressionado?

// Função para exibir mensagem no OLED
void display_message(char *text[], int num_lines) {  // Alterado para char *text[]
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    int y = 10; // Posição vertical inicial para o texto
    for (int i = 0; i < num_lines; i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);  // text[i] já é char *
        y += 8; // Espaçamento entre as linhas
    }

    render_on_display(ssd, &frame_area);
}

void SinalAberto() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    char *message[] = {  // Mantido como char *
        "SINAL ABERTO",
        "ATRAVESSAR",
        "COM CUIDADO"
    };
    display_message(message, 3);
}

void SinalFechado() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);

    char *message[] = {  // Mantido como char *
        "SINAL FECHADO",
        "AGUARDE"
    };
    display_message(message, 2);
}

int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i = i + 100) {
        A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1) {
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

int main() {
    stdio_init_all();

    // Inicialização do I2C para o OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do OLED
    ssd1306_init();

    // Inicialização dos LEDs
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Inicialização do Botão
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    while (true) {
        SinalAberto();
        A_state = WaitWithRead(8000); // Espera com leitura do botão

        if (A_state) { // Alguém apertou o botão - Sai do semáforo normal
            SinalFechado();
            sleep_ms(10000);
        } else { // Ninguém apertou o botão - Continua no semáforo normal
            SinalFechado();
            sleep_ms(8000);
        }
    }

    return 0;
}