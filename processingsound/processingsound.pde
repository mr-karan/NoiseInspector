import processing.serial.*;
// at the top of the program:
float xPos = 0; // horizontal position of the graph
float yPos = 0; // vertical position of the graph
Serial myPort;
void setup () {
  size(800, 600);        // window size
String portName = "/dev/ttyACM1";
myPort = new Serial(this, portName, 9600);
background(#4C0F1C);
}

void serialEvent (Serial myPort) {
  // get the byte:
  int inByte = myPort.read()*6;
  // print it:
  println(inByte);
  if (inByte==0){
    yPos=0;
  }
  yPos = height - inByte;
}

void draw () {
   // draw the line in a pretty color:
  stroke(#DAC0A7);
  line(xPos, height, xPos, yPos);
  // at the edge of the screen, go back to the beginning:
  if (xPos >= width) {
    xPos = 0;
    // clear the screen by resetting the background:
    background(#4C0F1C);
  } else {
    // increment the horizontal position for the next reading:
    xPos++;
  }
}