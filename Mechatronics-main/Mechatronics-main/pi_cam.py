# Import necessary libraries
import cv2
import numpy as np
import time
import csv
import serial
import keyboard

# Set up camera and image properties
CAMERA_DEVICE_ID = 0
IMAGE_WIDTH = 320
IMAGE_HEIGHT = 240
counter = 1

# Define color state and other flags
color_state = 1 
target_aquired = 0
manual = 0

# Define function for small motor commands, aiming
def motor_cmd_s(arg):
    # Reset input buffer for serial communication with small motor (ser_s)
    ser_s.reset_input_buffer()
    # Write command to the small motor
    ser_s.write(bytes(arg, 'utf-8'))
    # Read the acknowledgment response from the small motor
    line = ser_s.readline().decode('utf-8').rstrip()
    # Sleep for 1 second after the command
    time.sleep(1)

# Define function for medium motor commands, driving
def motor_cmd_m(arg):
    # Reset input buffer for serial communication with medium motor (ser_m)
    ser_m.reset_input_buffer()
    # Write command to the medium motor
    ser_m.write(bytes(arg, 'utf-8'))
    # Read the acknowledgment response from the medium motor
    line = ser_m.readline().decode('utf-8').rstrip()
    # Sleep for 1 second after the command
    time.sleep(1)
    
# Define function to send a command and wait for acknowledgment
def send_command_and_ack(ser, command, ack):
    # Reset input buffer for serial communication with given serial object (ser)
    ser.reset_input_buffer()
    # Write command to the serial object
    if command != 's':
        ser.write(bytes(command, 'utf-8'))
    # Wait until the acknowledgment response matches the provided ack parameter
        while True:
            test = ser.readline().decode('utf-8').rstrip()
            print(test)
            if test != ack:
                print("True")
                break


def target_aim(color_state):
    if color_state == 1: # this means we are targetting the TRIANGLE
        # Capture a frame from the webcam
        cap = cv2.VideoCapture(0)
        ret, frame = cap.read()
        # Save the captured frame as an image
        cv2.imwrite('shapes.jpg', frame)
        
        # Load the image
        image = cv2.imread('shapes.jpg')

        # Convert image to HSV color space
        hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        #green_pixel_rgb = (201,226,153)
        hsv_value = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        lower = hsv_value - (50,10,20)
        upper = hsv_value + (50,10,20)
        #print(hsv_value)
        # Define lower and upper bounds for green color in HSV
        lower_green = (30,100,100)
        upper_green = (80,215,215)
        #print(lower_green)
        #print(upper_green)

        # Create a binary mask for the green color
        green_mask = cv2.inRange(hsv_image, lower_green, upper_green)

        # Create a result image with only green pixels visible
        result_image = cv2.bitwise_and(image, image, mask=green_mask)

        # Save the result image
        cv2.imwrite("mask_file.jpg", result_image)

        img = cv2.imread("mask_file.jpg")

        # Convert the result image to grayscale
        gray_image = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # Apply thresholding to create a binary image
        ret, thresh = cv2.threshold(gray_image, 30, 255, 0)

        # Find contours in the binary mask
        contours, hierarchy = cv2.findContours(thresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)
        print("Number of contours detected:", len(contours))

        # Create a new blank image
        for cnt in contours:
            img = cv2.drawContours(img, [cnt], -1, (0, 255, 255), 3)

        # Compute the center of mass of the triangle
            M = cv2.moments(cnt)
            if M['m00'] != 0.0:
                x = int(M['m10'] / M['m00'])
                y = int(M['m01'] / M['m00'])
                # Draw a circle at the center of mass
                cv2.circle(img, (x, y), 5, 255, -1)
                print(x)
                print(y)
                target_aquired = 1
        # Display the image with contours and center of mass
        cv2.imshow("Shapes", img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

# Function to check the error in target alignment
def error_check(x, y):
    cx=x;
    cy=y;
    get_x = 0
    get_y = 0
    # Define target positions for the triangle (green), blue circle, and pins (not updated)
    if color_state == 1:
        target_x = 313
        target_y = 182
    
    
    # Calculate the distance
    if color_state == 1 or 2:
        # distance = np.sqrt((cx - target_x) ** 2 + (cy - target_y) ** 2)
        x_dist = abs(cx - target_x)
        y_dist = abs(cy - target_y)
        
        # If X-axis error is greater than 5, adjust position left or right
        if x_dist > 5:
            if cx > target_x:
                print("move right") #bot motion
                command = "r"
                time.sleep(2)
                motor_cmd_m(command)
            if cx < target_x:
                print("move left")
                command = "l"
                time.sleep(2)
                motor_cmd_m(command)
        else:
            print("You got X!")
            get_x = 1
        
        # If Y-axis error is greater than 5, adjust position up or down
        if y_dist > 5:
            if cy > target_y:
                print("move down") # ramp motion
                command = "d"
                time.sleep(1)
                motor_cmd_s(command)
            if cy < target_y:
                print("move up")
                time.sleep(1)
                motor_cmd_s(command)
        else:
            print("You got Y!")
            get_y = 1
    return(get_x, get_y)

# Main program execution
if __name__ == "__main__":
    # Set up serial communication with Arduino boards
    ser_s = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser_m = serial.Serial('/dev/ttyACM1', 9600, timeout=1)
    ser_s.reset_input_buffer()
    ser_m.reset_input_buffer()
    while True:
    # Target the green triangle
        if manual == 0: # Check if the target state is 1 (green target)
            time.sleep(3)  # Wait for 3 seconds before sending the command
            send_command_and_ack(ser_m, 's', 'Acks')  # wait for 'AckS' response
            
            # Send 'c' three times and wait for 'AckC' after each command
            for _ in range(3):
                print("Corner")
                send_command_and_ack(ser_m, 'c', 'Ackc')  # Wait for 'AckC' response
                
            # Send 'u' to Arduino and wait for 'Acku'
            for _ in range(4):
                print("Move up")
                send_command_and_ack(ser_s, 'u', 'Acku')  # Wait for 'AckUUUUU' response
            for _ in range(1):
                send_command_and_ack(ser_s, 'z', 'Ackz')
                
            x, y = None, None
            counter = 0
            while x and y == None and counter <= 2:
                x, y = target_aim(color_state)
                print("Corner")
                send_command_and_ack(ser_m, 'c', 'Ackc')  # Wait for 'AckC' response
                counter += 1
                
            while target_aquired == 0:
                # Start error check code
                # Note: This function might check the position of the target and adjust the position if needed
                x, y = target_aim(color_state) # Initialize x and y for error_check
                get_x, get_y = error_check(x, y) # Call error_check function and get x, y values
                
                # Check if both X and Y positions are matched (get_x and get_y are both 1)
                if error_check.get_x == 1 and error_check.get_y == 1:
                    print("Yeet")  # Success message for hitting the target
                    send_command_and_ack(ser_s, 'y', 'Acky')  # Wait for 'Acky' response
                    time.sleep(10)
            for _ in range(1):
                print("Reset")
                send_command_and_ack(ser_s, 'q', 'Ackq')  # Wait for 'Ackq' response
            
            for _ in range(3):  # Aiming at the pins
                print("Pivot Left")
                for _ in range(30):
                    send_command_and_ack(ser_m, 'L', 'AckL')
            
            for _ in range(1):
                print("little pivot left")
                for _ in range(10):
                    send_command_and_ack(ser_s, 'I', 'AckI')
                
            for _ in range(3):
                print("Move up")
                send_command_and_ack(ser_s, 'u', 'Acku')  # Wait for 'Ackuuu' response
            
            for _ in range(1):
                print("Yeet")  # Success message for hitting the pins
                send_command_and_ack(ser_s, 'y', 'Acky')  # Wait for 'Acky' response
                time.sleep(10)
            
            for _ in range(1):  # Aiming at the pins
                print("Pivot Left")
                for _ in range(30):
                    send_command_and_ack(ser_m, 'L', 'AckL')
            
            for _ in range(1):
                print("Reset")
                send_command_and_ack(ser_s, 'q', 'Ackq')  # Wait for 'Ackq' response
            
            for _ in range(1):
                print("Move down")
                send_command_and_ack(ser_s, 'd', 'Ackd')  # Wait for 'Ackd' response
            
            for _ in range(2):
                print("Yeet")  # Success message for hitting the pins
                send_command_and_ack(ser_s, 'y', 'Acky')  # Wait for 'Acky' response
                time.sleep(10)
                
        if manual == 1: #manual operation of robot
            cmd = ''
            cmd = input()
            if cmd == 'z':
                print("Move up a widdle bit")
                
                send_command_and_ack(ser_s, 'z', 'Ackz')
                
            if cmd == 'x':
                print("Move down a widdle bit")
            
                send_command_and_ack(ser_s, 'x', 'Ackx')
                
            if cmd == 'u':
                print("Move up")
                
                send_command_and_ack(ser_s, 'u', 'Acku')
                
            if cmd == 'd':
                print("Move down")
            
                send_command_and_ack(ser_s, 'd', 'Ackd')
                
            if cmd == 'y':
                print("Yeet")
        
                send_command_and_ack(ser_s, 'y', 'Acky')
            
            if cmd == 'r':
                print("raght")
            
                send_command_and_ack(ser_m, 'r', 'Ackr')
                
            if cmd == 'l':
                print("left")

                send_command_and_ack(ser_m, 'l', 'Ackl')
                
            if cmd == 'R':
                print("pivot right")
                for _ in range(20):
                    send_command_and_ack(ser_m, 'R', 'AckR')
            if cmd == 'L':
                print("pivot left")
                for _ in range(30):
                    send_command_and_ack(ser_m, 'L', 'AckL')
            if cmd == 'c':
                print("corner")
            
                send_command_and_ack(ser_m, 'c', 'Ackc')
            if cmd == 'q':
                print("reset")
            
                send_command_and_ack(ser_s, 'q', 'Ackq')
            if cmd == 'I':
                print("little pivot left")
                for _ in range(10):
                    send_command_and_ack(ser_s, 'I', 'AckI')
                