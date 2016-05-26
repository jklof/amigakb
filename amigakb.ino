/*
    Copyright 2016 Janne Lof

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*

For protocol spec see:
http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0173.html

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


class AmigaKb {
  
  enum { HI = 1, LO = 0 };

  byte klck;
  byte kdata;
  byte kbits; 

  byte get_klck()
  {
    // active high? seems like it is
    return digitalRead(PIN_KLCK) ? HI : LO;
  }
  byte get_kdat()
  {
    // active low +5V == 0, 0V == 1
    return digitalRead(PIN_KDAT) ? LO : HI;
  }
  void pulse_kdat()
  {
    // pulse low +5V at least for 85 micros
    pinMode(PIN_KDAT, OUTPUT);
    digitalWrite(PIN_KDAT, HIGH);
    delayMicroseconds(150);
    digitalWrite(PIN_KDAT, LOW);
    pinMode(PIN_KDAT, INPUT_PULLUP);
  }

public:

  void green(bool state)
  {
    digitalWrite(LED_GREEN, state ? HIGH : LOW);
  }
  void red(bool state)
  {
    digitalWrite(LED_RED, state ? HIGH : LOW);
  }

  // call this in setup()
  void init()
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
  int pollkey()
  {
    static const byte bitorder[] = { 6,5,4,3,2,1,0,7 };
    int rawkey = -1;
    
    byte k = get_klck();
    // only react if clock changes
    if ( k != klck ) {
      klck = k;
      // HI --> LO
      if ( klck == LO && kbits <= 7 ) {
        //Serial.print('*');
        kdata |= (get_kdat()<<bitorder[kbits]);
        kbits++;
      // LO --> HI
      } else if ( klck == HI && kbits > 7 ) {
        //Serial.print('+');
        rawkey = kdata;
        kbits = 0;
        kdata = 0;
        pulse_kdat();
      }
    }
    return rawkey;
  }

};


AmigaKb amigakb;

// the setup routine runs once when you press reset:
void setup() {              
  // initialize the digital pin as an output.
  //Serial.begin(9600);
  amigakb.init();
  amigakb.red(true);
  //Serial.println("started");
  
  Keyboard.begin();
  
}


static char keycode[0x78] = {
	[0]	 = '~', // grave
	[1]	 = '1',
	[2]	 = '2',
	[3]	 = '3',
	[4]	 = '4',
	[5]	 = '5',
	[6]	 = '6',
	[7]	 = '7',
	[8]	 = '8',
	[9]	 = '9',
	[10]	 = '0',
	[11]	 = '-', // minus
	[12]	 = '=', // equal
	[13]	 = '\\', //KEY_BACKSLASH,
        [14]     =  0,
	[15]	 = '0', //KEY_KP0,
	[16]	 = 'q',
	[17]	 = 'w',
	[18]	 = 'e',
	[19]	 = 'r',
	[20]	 = 't',
	[21]	 = 'y',
	[22]	 = 'u',
	[23]	 = 'i',
	[24]	 = 'o',
	[25]	 = 'p',
	[26]	 = '[', //KEY_LEFTBRACE,
	[27]	 = ']', //KEY_RIGHTBRACE,
        [28]     =  0,
	[29]	 = '1', //KEY_KP1,
	[30]	 = '2', //KEY_KP2,
	[31]	 = '3', //KEY_KP3,
	[32]	 = 'a',
	[33]	 = 's',
	[34]	 = 'd',
	[35]	 = 'f',
	[36]	 = 'g',
	[37]	 = 'h',
	[38]	 = 'j',
	[39]	 = 'k',
	[40]	 = 'l',
	[41]	 = ';', //KEY_SEMICOLON,
	[42]	 = '.', //KEY_APOSTROPHE,
	[43]	 = '\\', //KEY_BACKSLASH,
        [44]     =  0,
	[45]	 = '4', //KEY_KP4,
	[46]	 = '5', //KEY_KP5,
	[47]	 = '6', //KEY_KP6,
	[48]	 = '.', //KEY_102ND,
	[49]	 = 'z',
	[50]	 = 'x',
	[51]	 = 'c',
	[52]	 = 'v',
	[53]	 = 'b',
	[54]	 = 'n',
	[55]	 = 'm',
	[56]	 = ',', //KEY_COMMA,
	[57]	 = '.', //KEY_DOT,
	[58]	 = '/', //KEY_SLASH,
        [59]     = 0,
	[60]	 = '.', //KEY_KPDOT,
	[61]	 = '7', //KEY_KP7,
	[62]	 = '8', //KEY_KP8,
	[63]	 = '9', //KEY_KP9,
	[64]	 = ' ', //KEY_SPACE,
	[65]	 = KEY_BACKSPACE,
	[66]	 = KEY_TAB,
	[67]	 = KEY_RETURN, //KEY_KPENTER,
	[68]	 = KEY_RETURN, //KEY_ENTER,
	[69]	 = KEY_ESC,
	[70]	 = KEY_DELETE,
        [71]     = 0,
        [72]     = 0,
        [73]     = 0,
	[74]	 = '-', //KEY_KPMINUS,
        [75]     = 0,
	[76]	 = KEY_UP_ARROW, //KEY_UP,
	[77]	 = KEY_DOWN_ARROW, //KEY_DOWN,
	[78]	 = KEY_RIGHT_ARROW, //KEY_RIGHT,
	[79]	 = KEY_LEFT_ARROW, //KEY_LEFT,
	[80]	 = KEY_F1,
	[81]	 = KEY_F2,
	[82]	 = KEY_F3,
	[83]	 = KEY_F4,
	[84]	 = KEY_F5,
	[85]	 = KEY_F6,
	[86]	 = KEY_F7,
	[87]	 = KEY_F8,
	[88]	 = KEY_F9,
	[89]	 = KEY_F10,
	[90]	 = '[', //KEY_KPLEFTPAREN,
	[91]	 = ']', //KEY_KPRIGHTPAREN,
	[92]	 = '/', //KEY_KPSLASH,
	[93]	 = '*', //KEY_KPASTERISK,
	[94]	 = '+', //KEY_KPPLUS,
	[95]	 = KEY_F11, //KEY_HELP,
	[96]	 = KEY_LEFT_SHIFT,
	[97]	 = KEY_RIGHT_SHIFT,
	[98]	 = KEY_CAPS_LOCK,
	[99]	 = KEY_LEFT_CTRL,
	[100]	 = KEY_LEFT_ALT,
	[101]	 = KEY_RIGHT_ALT,
	[102]	 = KEY_LEFT_GUI, //KEY_LEFTMETA,
	[103]	 = KEY_RIGHT_GUI, //KEY_RIGHTMETA
};

void sendkeyevent(int rawkey)
{
  // special out of sync code
  if (rawkey == 0xF9) {
    Keyboard.releaseAll();
    return;
  }

  byte code = 0x7f & rawkey;
  // out of range 
  if (code >= sizeof(keycode)/sizeof(keycode[0]))
    return;
  // translate from amiga raw key codes  
  code = keycode[code]; 

  // no mapping from amiga codes..
  if (code == 0)
    return;

  // caps lock handling
  if (code == KEY_CAPS_LOCK) {
    Keyboard.press(code);
    Keyboard.release(code);
    return;    
  }


  if (rawkey & 0x80)
    Keyboard.release(code);
  else
    Keyboard.press(code);   

}


// the loop routine runs over and over again forever:
void loop() {
  
  int rawkey = amigakb.pollkey();
  if (rawkey != -1) {
    sendkeyevent(rawkey);    
  }

}
