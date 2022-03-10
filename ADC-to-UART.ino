#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define F_CPU 16000000ul        // Fosc
#define BAUD 9600ul           // BAUD rate
#define UBRR (F_CPU / (16 * BAUD) - 1)  // sample time, the signal gets sampled every UBRR ticks

void transmit(char* message, int n) {

  for(int i = 0; i < n; ++i) {
    // wait for the buffer to be empty
    while(!(UCSR0A & (1 << UDRE0))) {};
    
    //transmit the next character
    UDR0 = message[i];
  }
}

void Init_USART( void ) {
  // set BAUD rate through calculated UBRR
  UBRR0H = (uint8_t) (UBRR >> 8);
  UBRR0L = (uint8_t) UBRR;
  
  // Enable TX only
  UCSR0B = (1 << TXEN0);
  
  // Set frame format: 8data, 2stop bit
  UCSR0C = (1< < USBS0) | (3 << UCSZ00);
}

void Init_ADC( void ) {
  // Here you have to disable the digital input of a specific pin to use it as an ADC. In this case, the digital input op 'ADC0D' is disabled.
  DIDR0 |= (1 << ADC0D);

  // Here we configure ADC Multiplexer Selection register
  ADMUX |= (1 << REFS0) | (1 << ADLAR);

  // ADCSRA – ADC Control and Status Register A
  // Bit 7 – ADEN: ADC Enable
  // Bit 5 – ADATE: ADC Auto Trigger Enable - When this bit is written to one, Auto Triggering of the ADC is enabled. The ADC will start a conversion on a 
  // positive edge of the selected trigger signal. The trigger source is selected by setting the ADC Trigger Select bits, ADTS in ADCSRB
  // Bit 4 – ADIF: ADC Interrupt Flag - The ADC Conversion Complete Interrupt is executed if the ADIE bit and the I-bit in SREG are set.
  // Bit 3 – ADIE: ADC Interrupt Enable - When this bit is written to one and the I-bit in SREG is set, the ADC Conversion Complete Interrupt is activated.
  // Bits 2:0 – ADPS[2:0]: ADC Prescaler Select Bits - These bits determine the division factor between the system clock frequency and the input clock to the ADC.
  // ADSP2, ADPS1 and ADPS0 set to 1 configures the division factor between the system clock frequency and the input clock to the ADC to be 128 
  ADCSRA = (1 << ADPS2) | (1 << ADEN);
  ADCSRB |= (0 << ADPS2) | (0 << ADPS1) | (0 << ADPS0); // Here the ADC is in free running mode (see the corresponding table in the data sheet). You can also write 'ADCRSB = 0b0' 

}

int main(void)
{
  Init_USART();
  Init_ADC();
  
  while (1)
  {
    // execute a one time analog digital conversion
    ADCSRA |= (1 << ADSC);
    
    // wait for the ADC to finish conversion
    while(!(ADCSRA & (1 << ADIF))) {};

    char voltage_string[] = "Voltage: ";
    int voltage_string_n = sizeof(voltage_string) / sizeof(voltage_string[0]);
    
    // send a reading to the listening device
    transmit(voltage_string, voltage_string_n);
    
    // some overcomplicated way of converting a float to a char[]
    char voltage[5];
    dtostrf((float) ADCH / 255 * 5, 1, 2, &(voltage[0]));

    int voltage_n = sizeof(voltage) / sizeof(voltage[0]);

    transmit(voltage, voltage_n);

    char unit[] = "V\n";
    int unit_n = sizeof(unit) / sizeof(unit[0]);

    transmit(unit, unit_n);
  }
}
