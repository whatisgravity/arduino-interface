package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"io/ioutil"
	"log"
	"os"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/tarm/serial"
)

type ColorMessage struct {
	Red       int `json:"red"`
	Blue      int `json:"blue"`
	Green     int `json:"green"`
	FadeSpeed int `json:"fadeSpeed"`
}

type ResponseMessage struct {
	Data ColorMessage `json:"data"`
	Time int64        `json:"time"`
}

type Recent struct {
	ActiveMessages []ResponseMessage
}

var (
	help  = flag.Bool("help", false, "Provide a help dialog")
	debug = flag.Bool("debug", false, "Enable debug mode")
)

func main() {
	var serverName string
	flag.StringVar(&serverName, "name", "wall", "Provide a name for server configuration generation")
	var hostIP string
	flag.StringVar(&hostIP, "host", "0.0.0.0", "Provide an address to listen on")
	var portNumber string
	flag.StringVar(&portNumber, "--port", "8783", "Provide a port number")
	flag.Parse()
	if *help == true {
		log.Println("\n    ## Light Server")
		log.Println("      Usage:")
		log.Println("      Please specify the server defined in the configuration to start:")
		log.Println("          lsrvr [options]")
		log.Println("      Options:")
		log.Println("          --name  [option]   # Provide a name, if none default name (wall) will be used")
		log.Println("          --host  [option]   # Provide a interface, if none default interface (0.0.0.0) will be used")
		log.Println("          --port  [option]   # Provide a port, if none default port (8783) will be used\n")
		log.Println("          --debug [option]   # Debug mode, if none default value (false) will be used\n")
		log.Println("      Example:")
		log.Println("          lsrvr --host 10.0.0.5 --port 8080\n\n")
		os.Exit(0)
	}
	if *debug == false {
		os.Setenv("GIN_MODE", "release")
	}
	// Initialize Arduino
	serialDevice := findArduino()
	if serialDevice != "" {
		log.Println("Found arduino...", serialDevice)
	} else {
		log.Fatal("No arduino found.")
	}
	serialConfig := &serial.Config{Name: serialDevice, Baud: 9600}
	serialPort, err := serial.OpenPort(serialConfig)
	if err != nil {
		log.Fatal("Error: No serialport device found ", err)
	}
	defer serialPort.Close()
	// Initialize REST API
	recent := &Recent{[]ResponseMessage{}}
	r := gin.Default()
	r.GET("/", func(c *gin.Context) {
		c.JSON(200, recent.ActiveMessages)
	})
	r.POST("/", func(c *gin.Context) {
		var message ColorMessage
		err := c.BindJSON(&message)
		if err != nil {
			log.Println("Error: Failed to parse JSON ", err)
		}
		// Valdiate JSON Data
		if message.FadeSpeed < 1 {
			message.FadeSpeed = 1
		}
		message.FadeSpeed = checkLimits(message.FadeSpeed)
		message.Red = checkLimits(message.Red)
		message.Green = checkLimits(message.Green)
		message.Blue = checkLimits(message.Blue)
		// Pass Data To Arduino
		var responseColorMessage ColorMessage
		err = json.Unmarshal(passToSerial(serialPort, message), &responseColorMessage)
		var response = &ResponseMessage{
			Time: time.Now().Unix(),
			Data: responseColorMessage,
		}
		recent.ActiveMessages = append(recent.ActiveMessages, *response)
		// Serve Response
		c.JSON(200, response)
	})

	host := hostIP + ":" + portNumber
	log.Println("Light now listening on " + host + "\n")
	log.Println("Save the following to ~/.lctrl.yml if it does not already exist")
	log.Println("in order to use the lctrl command with this server.\n")
	log.Println("servers:")
	log.Println("  " + serverName + ":")
	log.Println("    host_ip: " + hostIP)
	log.Println("    host_port: " + portNumber)
	r.Run(host)
}

func checkLimits(value int) int {
	if value < 0 {
		return 0
	} else if value > 255 {
		return 255
	} else {
		return value
	}
}

func passToSerial(serialPort *serial.Port, message ColorMessage) []byte {
	// Write JSON Command
	messageBytes, err := json.Marshal(message)
	_, err = serialPort.Write(messageBytes)
	if err != nil {
		log.Println("Error: Write error", err)
		return nil
	}
	// Read JSON Response
	var outputBytes []byte
	var readCount int
	byteBuffer := make([]byte, 8)
	for {
		n, err := serialPort.Read(byteBuffer)
		if err != nil {
			log.Println("Error: Read error", err)
			break
		}
		readCount++
		outputBytes = append(outputBytes, byteBuffer[:n]...)
		if bytes.Contains(byteBuffer[:n], []byte("\n")) {
			// Stop at the termination of a JSON statement
			break
		} else if readCount > 15 {
			// Prevent from read lasting forever
			break
		}
	}
	return outputBytes
}

// findArduino looks for the file that represents the Arduino
// serial connection. Returns the fully qualified path to the
// device if we are able to find a likely candidate for an
// Arduino, otherwise an empty string if unable to find
// something that 'looks' like an Arduino device.
func findArduino() string {
	contents, _ := ioutil.ReadDir("/dev")
	// Look for what is mostly likely the Arduino device
	for _, f := range contents {
		if strings.Contains(f.Name(), "tty.usbserial") ||
			strings.Contains(f.Name(), "ttyUSB") ||
			strings.Contains(f.Name(), "ttyACM") {
			return "/dev/" + f.Name()
		}
	}
	// Have not been able to find a USB device that 'looks'
	// like an Arduino.
	return ""
}
