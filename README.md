# TextToGCode

When executed, Simply asks for text, then creates a Rasterised bitmap of the text.
It then turns this bitmap into a SVG.
Then the svg into Gcode.
Then loads the gcode and passes it to the first open serial port.

## Posible Command line arguments include:

```
-config [file] //Config file to modify gcode output
-text [sentance] //Text, must use _ between words.
-filename [name] //Name of the output file Default: test
-font [name] //Spesify the font to use. Default: uses random font from /fonts
-port number //Spesify the port to send the gcode to.
```

Waints for serial port to return "OK" after each line.

### TODO:
```
Make -config work
```