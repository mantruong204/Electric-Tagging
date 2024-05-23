# ELECTRONIC TAGGING
## Sample product photo
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/7556bd5d-d475-490c-9a7c-2cb0acdc9d3a)
# Functionalities:
Tracking position of user and warning if leaving the specified area with Ublox GY-NEO 6M
## Summary Description:
In the basic operating mode of the device, the current coordinates, including the longitude and latitude of the monitored person, the distance from the center, the date/month/year, and the real-time clock, are displayed on the OLED screen. Additionally, the distance from the center is constantly monitored and compared to a predefined radius R (in meters). If this distance exceeds R, indicating that the monitored person has left the allowed area, an alarm will be activated and a warning will be issued.

## Basic System Flow:
For each frame received from the GPS module via UART, the encoded information in the transmitted frame will be extracted and displayed on the OLED. The current longitude and latitude information, along with the pre-set center area's coordinates, will be fed into a distance calculation function. The result is then compared to the allowed radius R to decide whether to trigger an alarm.

## Function to Update the Center Area Coordinates:

**Summary Description:**
By pressing and holding the Lock Centroid button for 2 seconds, the current coordinates will be updated to become the center area coordinates.

**Basic System Flow:**
This use case is initiated when the user presses and holds the Lock Centroid button for about 2 seconds. When powered on, the device operates in basic mode. If the user holds the Lock Centroid button for about 2 seconds, the variables storing the center area coordinates will be updated with the device's current coordinates. The OLED screen will display the message "CENTROID LOCKED" along with the new center area coordinates for 2 seconds. After these 2 seconds, the system will return to normal operation mode, and the OLED screen will revert to the normal interface.

# Block Diagram
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/a3690ded-fc1c-4265-b3fb-f6b300449009)

# Schematic Design
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/05eb6a79-1709-4467-b22b-dcaea2477597)

# PCB Design
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/beabdf85-211c-4896-83ed-df3946b6a9d3)

# Flowchart
## Generic flow
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/96619984-7119-4782-b71d-1ffbe38aee0b)
## Timer Interrupt Process
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/51961a1e-64f0-4edd-8b2f-cad6db116cc9)

# Hardware
## PCB Product
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/40d7db82-4d83-4769-8e7e-1a7db1f3b384)
## Final product
![ảnh](https://github.com/mantruong204/Electric-Tagging/assets/155959855/e1ab0c37-2ab0-4285-92c8-00a06817abaa)

# Demonstration
[![Video Title](https://img.youtube.com/vi/r7t7nPDNbt0/0.jpg)](https://youtu.be/r7t7nPDNbt0)
