// License: GNU GPL 3
// 
// For protocol spec see:
// http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0173.html


/*

The keyboard transmits 8-bit data words serially to the main unit. Before
the transmission starts, both KCLK and KDAT are high.  The keyboard starts
the transmission by putting out the first data bit (on KDAT), followed by
a pulse on KCLK (low then high); then it puts out the second data bit and
pulses KCLK until all eight data bits have been sent.  After the end of
the last KCLK pulse, the keyboard pulls KDAT high again.

When the computer has received the eighth bit, it must pulse KDAT low for
at least 1 (one) microsecond, as a handshake signal to the keyboard.  The
handshake detection on the keyboard end will typically use a hardware
latch.  The keyboard must be able to detect pulses greater than or equal
to 1 microsecond.  Software MUST pulse the line low for 85 microseconds to
ensure compatibility with all keyboard models.

All codes transmitted to the computer are rotated one bit before
transmission.  The transmitted order is therefore 6-5-4-3-2-1-0-7. The
reason for this is to transmit the  up/down flag  last, in order to cause
a key-up code to be transmitted in case the keyboard is forced to restore
 lost sync  (explained in more detail below).

The KDAT line is active low; that is, a high level (+5V) is interpreted as
0, and a low level (0V) is interpreted as 1.

             _____   ___   ___   ___   ___   ___   ___   ___   _________
        KCLK      \_/   \_/   \_/   \_/   \_/   \_/   \_/   \_/
             ___________________________________________________________
        KDAT    \_____X_____X_____X_____X_____X_____X_____X_____/
                  (6)   (5)   (4)   (3)   (2)   (1)   (0)   (7)

                 First                                     Last
                 sent                                      sent

The keyboard processor sets the KDAT line about 20 microseconds before it
pulls KCLK low.  KCLK stays low for about 20 microseconds, then goes high
again.  The processor waits another 20 microseconds before changing KDAT.


keyboard connector:
KLCK    <--black------ 0
KDAT    <--brown------ 1
KBRESET <--red-------- 2 (resets amiga)
+5V     <--orange------3
KEY     <--yellow------4 (not used)
GROUND  <--green-------5
STATUS  <--blue--------6 (disk drive led)
INUSE   <--violet------7 (power led)


*/

// arduino pin mappings
#define LED_GREEN 6 // 'in use' violet
#define LED_RED   7 // 'status' blue
#define PIN_KLCK  8  // black
#define PIN_KDAT  9  // brown


#define HI 1
#define LO 0

unsigned klck = HI;
unsigned kdata = 0;
unsigned kbits = 0;

inline unsigned get_klck()
{
    // active high? seems like it is
    return digitalRead(PIN_KLCK) ? HI : LO;
}
inline unsigned  get_kdat()
{
    // active low +5V == 0, 0V == 1
    return digitalRead(PIN_KDAT) ? LO : HI;
}
inline void pulse_kdat()
{
    //Serial.print('+');
    // pulse low +5V at least for 85 micros
    pinMode(PIN_KDAT, OUTPUT);
    digitalWrite(PIN_KDAT, HIGH);
    delayMicroseconds(120);
    digitalWrite(PIN_KDAT, LOW);
    pinMode(PIN_KDAT, INPUT_PULLUP);
}

void amiga_kb_green(bool state)
{
  digitalWrite(LED_GREEN, state ? HIGH : LOW);
}
void amiga_kb_red(bool state)
{
  digitalWrite(LED_RED, state ? HIGH : LOW);
}

// call this in setup()
void amiga_kb_init()
{
    klck = HI;
    kdata = 0;
    kbits = 0;
    pinMode(PIN_KLCK, INPUT_PULLUP);
    pinMode(PIN_KDAT, INPUT_PULLUP);
    pinMode(LED_RED,   OUTPUT);     
    pinMode(LED_GREEN, OUTPUT);
}

// call this in loop(), returns raw keycode or -1 if no key was read
int amiga_kb_loop()
{
    static const int bitorder[] = { 6,5,4,3,2,1,0,7 };
    int ret = -1;
    
    /* latch, only react if klck changes */
    unsigned k = get_klck();
    if ( k == klck )
        return -1;
    klck = k;

    /* HI --> LO */
    if ( klck == LO && kbits <= 7) {
        //Serial.print('*');
        kdata |= (get_kdat()<<bitorder[kbits]);
        kbits++;
    /* LO --> HI */
    } else if ( klck == HI && kbits > 7) {
        //Serial.print('*');
        ret = kdata;
        kbits = 0;
        kdata = 0;
        pulse_kdat();
    }
    return ret;
}




// the setup routine runs once when you press reset:
void setup() {              
  // initialize the digital pin as an output.
  Serial.begin(9600);
  amiga_kb_init();
  amiga_kb_red(true);
  Serial.println("started");
}

// the loop routine runs over and over again forever:
void loop() {
  
  int rawkey = amiga_kb_loop();
  if (rawkey != -1) {
    
    if (rawkey & 0x80) {
      Serial.print("up   ");
      amiga_kb_green(false);
    } else {
      Serial.print("down ");
      amiga_kb_green(true);
    }
    Serial.print(rawkey, HEX);
    Serial.println();
    
  }

}
