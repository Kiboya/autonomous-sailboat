# Installing PlatformIO and Setting Up Raspberry Pi Pico with VS Code

## Prerequisites
- Install the **PlatformIO** extension in VS Code
- Install **Minicom** if you haven’t already

## Importing the Arduino Project
1. Click on the **ant icon** in the sidebar
2. In the window that opens (orange ant), select **Import Arduino Project**
3. Choose the **Raspberry Pi Pico** board
4. Check the option **Use Arduino Libraries**
5. Select the project folder to open
6. In the top bar of VS Code, run:
   ```
   > Reload Window
   ```
7. Grab a coffee while the installation completes—it might take a few minutes

## Enabling the Serial Monitor
To open the serial monitor, in the top bar of VS Code, run:
```
> PlatformIO: Serial Monitor
```

## Building & Uploading
- **Build** your project
- To upload the code, ensure the board has not been flashed previously.
  - Unplug the board
  - Hold the **BOOTSEL** button
