#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define PINO_ALARME 12
#define FREQ_ALARME 2000
// É NECESSARIO APERTAR E SEGURAR O BOTÃO
const uint PINO_SINAL_PARAR = 0;      // LED Vermelho
const uint PINO_SINAL_ATENCAO = 1;    // LED Amarelo
const uint PINO_SINAL_SEGUIR = 2;     // LED Verde
const uint PINO_SINAL_ATRAVESSAR = 3; // LED Verde pedestre
const uint PINO_ACIONADOR = 15;       // Botão

void configurar_alarme(uint pino) {
    gpio_set_function(pino, GPIO_FUNC_PWM);
    uint num_fatia = pwm_gpio_to_slice_num(pino);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, clock_get_hz(clk_sys) / (FREQ_ALARME * 4096));
    pwm_init(num_fatia, &cfg, true);
    pwm_set_gpio_level(pino, 0);
}

void emitir_som(uint pino, uint duracao) {
    uint num_fatia = pwm_gpio_to_slice_num(pino);
    
    for (int i = 0; i < duracao; i++) {
        pwm_set_gpio_level(pino, 2048);
        sleep_ms(500);
        pwm_set_gpio_level(pino, 0);
        sleep_ms(500);
    }
    
    sleep_ms(100);
}

void acionar_sinal(uint pino, uint duracao) {
    gpio_put(pino, 1);
    sleep_ms(duracao * 1000);
    gpio_put(pino, 0);
}

void modo_travessia(uint duracao) {
    gpio_put(PINO_SINAL_ATRAVESSAR, 1);
    gpio_put(PINO_SINAL_PARAR, 1);
    emitir_som(PINO_ALARME, duracao);
    gpio_put(PINO_SINAL_ATRAVESSAR, 0);
    gpio_put(PINO_SINAL_PARAR, 0);
}

int main() {
    // Inicialização dos pinos
    const uint pinos[] = {PINO_SINAL_PARAR, PINO_SINAL_ATENCAO, PINO_SINAL_SEGUIR, 
                         PINO_SINAL_ATRAVESSAR, PINO_ALARME, PINO_ACIONADOR};
    
    for(int i = 0; i < 6; i++) {
        gpio_init(pinos[i]);
        gpio_set_dir(pinos[i], i == 5 ? GPIO_IN : GPIO_OUT);
        if(i < 4) gpio_put(pinos[i], 0);  // Desliga LEDs inicialmente
    }

    gpio_pull_up(PINO_ACIONADOR);
    configurar_alarme(PINO_ALARME);

    while (true) {
        acionar_sinal(PINO_SINAL_SEGUIR, 8);
        
        if (!gpio_get(PINO_ACIONADOR)) {
            acionar_sinal(PINO_SINAL_ATENCAO, 5);
            modo_travessia(15);
        } else {
            acionar_sinal(PINO_SINAL_ATENCAO, 2);
            acionar_sinal(PINO_SINAL_PARAR, 10);
        }
    }
    return 0;
}