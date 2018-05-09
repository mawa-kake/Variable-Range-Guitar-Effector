#define LENGTH 256

byte rawData[LENGTH];
int count;

// Sample Frequency in kHz
const float sample_freq = 8919;

int len = sizeof(rawData);
int i,k;
long sum, sum_old;
int thresh = 0;
float freq_per = 0;
// For eliminating harmonics
float freq_old = 0;
byte pd_state = 0;
// Variable to keep writing 1 or 0 to serial even when stops reading signal
int keep_writing = 0;

void setup(){
  analogReference(EXTERNAL);   // Connect to 3.3V
  analogRead(A0);
  Serial.begin(115200);
  count = 0;
}


void loop(){  
  if (count < LENGTH) {
    count++;
    rawData[count] = analogRead(A0)>>2;
  }
  else {
    sum = 0;
    pd_state = 0;
    int period = 0;
    for(i=0; i < len; i++)
    {
      // Autocorrelation
      sum_old = sum;
      sum = 0;
      for(k=0; k < len-i; k++)
	    sum += (rawData[k]-128)*(rawData[k+i]-128)/256;
        //Serial.println(sum);
      
      // Peak Detect State Machine
      if (pd_state == 2 && (sum-sum_old) <=0) 
      {
        period = i;
        //Serial.println(period);
        pd_state = 3;
      }
      if (pd_state == 1 && (sum > thresh) && (sum-sum_old) > 0)
        pd_state = 2;
      if (!i) {
        thresh = sum * 0.5;
        pd_state = 1;
      }
    }
    
    // Frequency identified in Hz
    if (thresh > 100) {
      // New frequency
      freq_per = sample_freq/period;

      // If current = (2*previous +- 1) or (3*previous +-1), use last frequency
      // Otherwise use new frequency
      if ((freq_per >= (2*freq_old-10) && freq_per <= (2*freq_old+10)) || (freq_per >= (3*freq_old-10) && freq_per <= (3*freq_old+10)))
        freq_per = freq_old;
    }

    //Writing to Pure Data
    if (freq_per > 100000 ) {
    //This is to ignore infinity readings
    }

    // For now, if it's 1, keep writing 1 until a new frequency is read (don't reset back to 0)

    //feel free to change 150 to whatever is easier to test
    else if (freq_per > 150) {
      //Serial.write(1);
      keep_writing = 1;
      //Serial.println("Writing 1");
      Serial.println(freq_per);
    }
    else {
      //Serial.write((byte)0);
      keep_writing = 0;
      Serial.println(freq_per);
      //Serial.println("Writing 0");
    }

    if (keep_writing == 1)
    {
      Serial.write(1);
      //Serial.println("Writing 1");
    }

    if (keep_writing == 0)
    {
      Serial.write((byte)0);
      //Serial.println("Writing 1");
    }
    
    // Save last used frequency
    freq_old = freq_per;
    count = 0;
  }
}