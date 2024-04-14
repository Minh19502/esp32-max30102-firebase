
#include <Servo.h>
#include <AFMotor.h>

#define LINE_BUFFER_LENGTH 512

char STEP = MICROSTEP ;

// Vị trí servo cho Lên và Xuống
const int penZUp = 95;
const int penZDown = 75;

//servo trên chân xung 10
const int penServoPin =10 ;

// Có lẽ phù hợp với các bước DVD, nhưng ở đây không quá quan trọng
const int stepsPerRevolution = 48; 

// tạo đối tượng servo để điều khiển servo 
Servo penServo;  

// Initialize steppers for X- and Y-axis using this Arduino pins for the L293D H-bridge
AF_Stepper myStepperY(stepsPerRevolution,1);            
AF_Stepper myStepperX(stepsPerRevolution,2);  

/* Structures, global variables    */
struct point { 
  float x; 
  float y; 
  float z; 
};

// Vị trí hiện tại của chủ lô
struct point actuatorPos;

// Cài đặt vẽ,và ổn định;
float StepInc = 1;
int StepDelay = 2;
int LineDelay =0;
int penDelay = 50;

// Động cơ bước đi 1 mm.
// Sử dụng bản phác thảo thử nghiệm để đi 100 bước. Đo chiều dài của dòng. 
// Tính toán các bước trên mỗi mm. Nhập vào đây.
float StepsPerMillimeterX = 100.0;
float StepsPerMillimeterY = 100.0;

//Vẽ giới hạn robot, tính bằng mm
// OK để bắt đầu. Có thể lên tới 50 mm nếu được hiệu chỉnh tốt. 
float Xmin = 0;
float Xmax = 35;
float Ymin = 0;
float Ymax = 35;
float Zmin = 0;
float Zmax = 1;

float Xpos = Xmin;
float Ypos = Ymin;
float Zpos = Zmax; 

// Đặt thành true để nhận kết quả gỡ lỗi.
boolean verbose = false;

//  Cần giải nghĩa;
//  G1 for moving
//  G4 P300 (wait 150ms)
//  M300 S30 (pen down)
//  M300 S50 (pen up)
//  Loại bỏ bất cứ thứ gì có (
// Loại bỏ bất kỳ lệnh nào khác!

/**********************
 * void setup() - Initialisations
 ***********************/
void setup() {
   //  Cài đặt
  
  Serial.begin( 9600 );
  
  penServo.attach(penServoPin);
  penServo.write(penZUp);
  delay(100);

   // Giảm nếu cần thiết
  myStepperX.setSpeed(250);

  myStepperY.setSpeed(250);  
  

 //  Đặt & di chuyển về vị trí mặc định ban đầu
  // TBD

//  Thông báo!!!
  Serial.println("Mini CNC Plotter alive and kicking!");
  Serial.print("X range is from "); 
  Serial.print(Xmin); 
  Serial.print(" to "); 
  Serial.print(Xmax); 
  Serial.println(" mm."); 
  Serial.print("Y range is from "); 
  Serial.print(Ymin); 
  Serial.print(" to "); 
  Serial.print(Ymax); 
  Serial.println(" mm."); 
}

/**********************
 * void loop() - Main loop
 ***********************/
void loop() 
{
  
  delay(100);
  char line[ LINE_BUFFER_LENGTH ];
  char c;
  int lineIndex;
  bool lineIsComment, lineSemiColon;

  lineIndex = 0;
  lineSemiColon = false;
  lineIsComment = false;

  while (1) {

    // Nhận dữ liệu chuỗi - Chủ yếu từ Grbl, đã thêm hỗ trợ dấu chấm phẩy.
    while ( Serial.available()>0 ) {
      c = Serial.read();
      if (( c == '\n') || (c == '\r') ) {             // Đã đến cuối dòng.
        if ( lineIndex > 0 ) {                        // Dòng lệnh đã hoàn chỉnh. Tiến hành thực hiện!
          line[ lineIndex ] = '\0';                   // Chấm dứt chuỗi.
          if (verbose) { 
            Serial.print( "Received : "); 
            Serial.println( line ); 
          }
          processIncomingLine( line, lineIndex );
          lineIndex = 0;
        } 
        else { 
           // Dòng trống hoặc dòng chú thích. Bỏ qua khối lệnh đó.
        }
        lineIsComment = false;
        lineSemiColon = false;
        Serial.println("ok");    
      } 
      else {
        if ( (lineIsComment) || (lineSemiColon) ) {   // Loại bỏ tất cả các ký tự chú thích.
          if ( c == ')' )  lineIsComment = false;     // Kết thúc chú thích. Tiếp tục dòng lệnh.
        } 
        else {
          if ( c <= ' ' ) {                           // Loại bỏ các khoảng trắng và ký tự điều khiển.
          } 
          else if ( c == '/' ) {                    // Không hỗ trợ xóa khối. Bỏ qua ký tự đó.
          } 
          else if ( c == '(' ) {                    // Bật cờ chú thích và bỏ qua tất cả các ký tự cho đến khi gặp ')' hoặc kết thúc dòng.
            lineIsComment = true;
          } 
          else if ( c == ';' ) {
            lineSemiColon = true;
          } 
          else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
            Serial.println( "ERROR - lineBuffer overflow" );
            lineIsComment = false;
            lineSemiColon = false;
          } 
          else if ( c >= 'a' && c <= 'z' ) {        // Viết hoa viết thường
            line[ lineIndex++ ] = c-'a'+'A';
          } 
          else {
            line[ lineIndex++ ] = c;
          }
        }
      }
    }
  }
}

void processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  char buffer[ 64 ];                                 //  64 là đủ cho 1 tham số
  struct point newPos;

  newPos.x = 0.0;
  newPos.y = 0.0;

  //  Needs to interpret 
  //  G1 để di chuyển
  //  G4 P300 (đợi 150ms)
  //  G1 X60 Y30
  //  G1 X30 Y50
  //  M300 S30 (bút nâng lên)
  //  M300 S50 (bút hạ xuống)
  //  Discard anything with a (
  //  Discard any other command!

  while( currentIndex < charNB ) {
    switch ( line[ currentIndex++ ] ) {              // Select command, if any
    case 'U':
      penUp(); 
      break;
    case 'D':
      penDown(); 
      break;
    case 'G':
      buffer[0] = line[ currentIndex++ ];          // /!\ Dirty - chỉ hoạt động với con số 2 chữ số
      //      buffer[1] = line[ currentIndex++ ];
      //      buffer[2] = '\0';
      buffer[1] = '\0';

      switch ( atoi( buffer ) ){                   // chọn lệnh G
      case 0:                                   // G00 & G01 - chuyển động hoặc chuyển động nhanh, như nhau
      case 1:
         // /!\ Dirty - giẢ sử X đứng trước Y
        char* indexX = strchr( line+currentIndex, 'X' );   // vị trí X/Y trong chuỗi(nếu có)
        char* indexY = strchr( line+currentIndex, 'Y' );
        if ( indexY <= 0 ) {
          newPos.x = atof( indexX + 1); 
          newPos.y = actuatorPos.y;
        } 
        else if ( indexX <= 0 ) {
          newPos.y = atof( indexY + 1);
          newPos.x = actuatorPos.x;
        } 
        else {
          newPos.y = atof( indexY + 1);
          indexY = nullptr;
          newPos.x = atof( indexX + 1);
        }
        drawLine(newPos.x, newPos.y );
        //        Serial.println("ok");
        actuatorPos.x = newPos.x;
        actuatorPos.y = newPos.y;
        break;
      }
      break;
    case 'M':
      buffer[0] = line[ currentIndex++ ];       // /!\ Dirty - chỉ hoạt động với lệnh 3 chữ số
      buffer[1] = line[ currentIndex++ ];
      buffer[2] = line[ currentIndex++ ];
      buffer[3] = '\0';
      switch ( atoi( buffer ) ){
      case 300:
        {
          char* indexS = strchr( line+currentIndex, 'S' );
          float Spos = atof( indexS + 1);
          //         Serial.println("ok");
          if (Spos == 30) { 
            penDown(); 
          }
          if (Spos == 50) { 
            penUp(); 
          }
          break;
        }
      case 114:                               // M114 - báo cáo vị trí
        Serial.print( "Absolute position : X = " );
        Serial.print( actuatorPos.x );
        Serial.print( "  -  Y = " );
        Serial.println( actuatorPos.y );
        break;
      default:
        Serial.print( "Command not recognized : M");
        Serial.println( buffer );
      }
    }
  }



}


/*********************************
 * Draw a line from (x0;y0) to (x1;y1). 
 * Bresenham algo from https://www.marginallyclever.com/blog/2013/08/how-to-build-an-2-axis-arduino-cnc-gcode-interpreter/
 * int (x1;y1) : Starting coordinates
 * int (x2;y2) : Ending coordinates
 **********************************/
void drawLine(float x1, float y1) {

  if (verbose)
  {
    Serial.print("fx1, fy1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }  

    // đưa hướng dẫn vào trong giới hạn
  if (x1 >= Xmax) { 
    x1 = Xmax; 
  }
  if (x1 <= Xmin) { 
    x1 = Xmin; 
  }
  if (y1 >= Ymax) { 
    y1 = Ymax; 
  }
  if (y1 <= Ymin) { 
    y1 = Ymin; 
  }

  if (verbose)
  {
    Serial.print("Xpos, Ypos: ");
    Serial.print(Xpos);
    Serial.print(",");
    Serial.print(Ypos);
    Serial.println("");
  }

  if (verbose)
  {
    Serial.print("x1, y1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }

   //  chuyển đổi toạ độ sang các bước
  x1 = (int)(x1*StepsPerMillimeterX);
  y1 = (int)(y1*StepsPerMillimeterY);
  float x0 = Xpos;
  float y0 = Ypos;

//  cùng tìm hiểu sự thay đôir của toạ độ
  long dx = abs(x1-x0);
  long dy = abs(y1-y0);
  int sx = x0<x1 ? StepInc : -StepInc;
  int sy = y0<y1 ? StepInc : -StepInc;

  long i;
  long over = 0;

  if (dx > dy) {
    for (i=0; i<dx; ++i) {
      myStepperX.onestep(sx,STEP);
      over+=dy;
      if (over>=dx) {
        over-=dx;
        myStepperY.onestep(sy,STEP);
      }
    delay(StepDelay);
    }
  }
  else {
    for (i=0; i<dy; ++i) {
      myStepperY.onestep(sy,STEP);
      over+=dx;
      if (over>=dy) {
        over-=dy;
        myStepperX.onestep(sx,STEP);
      }
      delay(StepDelay);
    }    
  }

  if (verbose)
  {
    Serial.print("dx, dy:");
    Serial.print(dx);
    Serial.print(",");
    Serial.print(dy);
    Serial.println("");
  }

  if (verbose)
  {
    Serial.print("Going to (");
    Serial.print(x0);
    Serial.print(",");
    Serial.print(y0);
    Serial.println(")");
  }

//  Đợi một khoảng thời gian trước khi gửi bất kỳ dòng văn bản tiếp theo.

  delay(LineDelay);
  //  Update the positions
  Xpos = x1;
  Ypos = y1;
}

//  nâng bút
void penUp() { 
  penServo.write(penZUp); 
  delay(penDelay); 
  Zpos=Zmax; 
  digitalWrite(15, LOW);
    digitalWrite(16, HIGH);
  if (verbose) { 
    Serial.println("Pen up!"); 
    
  } 
}
//  hạ bút
void penDown() { 
  penServo.write(penZDown); 
  delay(penDelay); 
  Zpos=Zmin; 
  digitalWrite(15, HIGH);
    digitalWrite(16, LOW);
  if (verbose) { 
    Serial.println("Pen down."); 
    
    
  } 
}
