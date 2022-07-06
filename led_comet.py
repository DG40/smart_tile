import time
import serial
from random import seed
from random import randint
from rpi_ws281x import *

# LEDs order:
order = [
	[56, 55, 40, 39, 24, 23, 8, 7, 248, 247, 232, 231, 216, 215, 200, 199], 
	[57, 54, 41, 38, 25, 22, 9, 6, 249, 246, 233, 230, 217, 214, 201, 198], 
	[58, 53, 42, 37, 26, 21, 10, 5, 250, 245, 234, 229, 218, 213, 202, 197], 
	[59, 52, 43, 36, 27, 20, 11, 4, 251, 244, 235, 228, 219, 212, 203, 196], 
	[60, 51, 44, 35, 28, 19, 12, 3, 252, 243, 236, 227, 220, 211, 204, 195], 
	[61, 50, 45, 34, 29, 18, 13, 2, 253, 242, 237, 226, 221, 210, 205, 194], 
	[62, 49, 46, 33, 30, 17, 14, 1, 254, 241, 238, 225, 222, 209, 206, 193], 
	[63, 48, 47, 32, 31, 16, 15, 0, 255, 240, 239, 224, 223, 208, 207, 192], 
	[64, 79, 80, 95, 96, 111, 112, 127, 128, 143, 144, 159, 160, 175, 176, 191], 
	[65, 78, 81, 94, 97, 110, 113, 126, 129, 142, 145, 158, 161, 174, 177, 190], 
	[66, 77, 82, 93, 98, 109, 114, 125, 130, 141, 146, 157, 162, 173, 178, 189], 
	[67, 76, 83, 92, 99, 108, 115, 124, 131, 140, 147, 156, 163, 172, 179, 188], 
	[68, 75, 84, 91, 100, 107, 116, 123, 132, 139, 148, 155, 164, 171, 180, 187], 
	[69, 74, 85, 90, 101, 106, 117, 122, 133, 138, 149, 154, 165, 170, 181, 186], 
	[70, 73, 86, 89, 102, 105, 118, 121, 134, 137, 150, 153, 166, 169, 182, 185], 
	[71, 72, 87, 88, 103, 104, 119, 120, 135, 136, 151, 152, 167, 168, 183, 184]
	]

# Size of LED-panel:	
size_y = len(order)
size_x = len(order[0])

# LED strip configuration:
LED_COUNT      = 256      # Number of LED pixels.
LED_PIN        = 18      # GPIO pin connected to the pixels (must support PWM!).
LED_FREQ_HZ    = 650000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10      # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0
LED_STRIP      = ws.SK6812_STRIP_GRBW

grid = []	# - for adding tail to comet
for _ in range(size_y):
	grid.append([0] * size_x)
smooth = 15	# - coefficient for comet tail length

# Reading data from joystick tile:
def readData():
	com.write(b"Z\n")
	response = com.readline()
	try:
		d = response.decode("utf-8").strip()
		if d == 'W': # Without human:
			x, y = '0', '0'
		elif len(d) >= 4: # With human:
			x = d[d.index('X') + 1:d.index('Y')]
			y = d[d.index('Y') + 1:]
		else: # Missed DATA:
			print('Missed DATA')
			x, y = '0', '0'
	except UnicodeDecodeError: # Error in transaction:
		print('Incorrect DATA')
		x, y = '0', '0'
	try:
		x, y = int(x), int(y)
		x = round(x * (size_x / 14))
		y = round(y * (size_y / 14))
		if x > size_x:
			x = size_x
		elif x < 0:
			x = 0
		if y > size_y:
			y = size_y
		elif y < 0:
			y = 0
	except ValueError:
		pass
	return x, y


def clear_leds():
	for i in range(strip.numPixels()):
		strip.setPixelColor(i, Color(0, 0, 0, 0))


def fill_leds(r:int=10, g:int=10, b:int=10, w:int=10):
	for i in range(size_x):
		for j in range(size_y):
			strip.setPixelColor(order[i][j], Color(r, g, b, w))


def refresh_grid(x: int, y: int, radius:int=3, raising:int=5, fading:int=5):
	
	for i in range(size_y):
		for j in range(size_x):
			if (i - x) ** 2 + (j - y) ** 2 <= radius ** 2: # Esli v radiuse
				if i == x and j == y and grid[i][j] + raising < 255: # Esli center
					grid[i][j] += raising
				else: # Ostal'nie v raduise:
					for r in range(1, radius + 1):
						if (r - 1) ** 2 <= (i - x) ** 2 + (j - y) ** 2 <= r ** 2:
							addition = raising * (radius - r) // radius
							if grid[i][j] + addition < 255:
								grid[i][j] += addition
			else:
				if grid[i][j] - fading < 0: 
					grid[i][j] = 0
				else:
					grid[i][j] -= fading
			strip.setPixelColor(order[i][j], Color(grid[i][j], grid[i][j], 0, grid[i][j]))

		
def fade_comet(fading:int=5):
	for i in range(size_y):
		for j in range(size_x):
			if grid[i][j] - fading < 0: 
				grid[i][j] = 0
			else:
				grid[i][j] -= fading
			strip.setPixelColor(order[i][j], Color(grid[i][j], grid[i][j], 0, grid[i][j]))

	
if __name__ == '__main__':

	##############
	### SETUP: ###
	##############

	com = serial.Serial('/dev/ttyS0', 115200, timeout=1)
	com.flush()

	strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL, LED_STRIP)
	strip.begin()
	clear_leds()
	strip.show()

	print ('Press Ctrl-C to quit.')

	last_x, last_y = 0, 0 # Comet coordinates
	while last_x <= 0 and last_y <= 0:
		last_x, last_y = readData()
	print(last_x, last_y)

	print('Let it go')

	#############
	### LOOP: ###
	#############

	while True:
		new_x, new_y = readData()
		if (new_x, new_y) == (0, 0):
			for _ in range(smooth):
				fade_comet(fading=2)
				strip.show()
			new_x = size_x // 2
			new_y = size_y // 2
		elif last_x < new_x:
			k = (new_y - last_y) / (new_x - last_x)
			b = last_y - last_x * k
			for x in range(last_x, new_x + 1):
				for _ in range(smooth):
					refresh_grid(x, round(k * x + b), raising=10, fading=1)
					strip.show()
		elif last_x > new_x:
			k = (new_y - last_y) / (new_x - last_x)
			b = last_y - last_x * k
			for x in range(last_x, new_x + 1, -1):
				
				for _ in range(smooth):
					refresh_grid(x, round(k * x + b), raising=10, fading=1)
					strip.show()
		else:
			for _ in range(smooth):
				refresh_grid(new_x, new_y, raising=10, fading=5)
				strip.show()
		
		last_x, last_y = new_x, new_y
