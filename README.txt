cs58 max_z

compilation and clean:
run “make” to compile
run “make clean” to delete all outputs from compilation

run program:
album ./pathtorawimage1 ./pathtorawimage2 ….. ./pathtorawimagex
if folder (say it’s called photos) of images is in directory
	album ./photos/*.jpg

output:
leaves the following in directory
	properly oriented thumbnail of images in args
	properly oriented medium-size version of images in args
	index.html
		header, properly oriented thumbnail(s), caption(s), and link from 			thumbnail to medium-size version (all properly oriented)

files:
album.c
	main and helper functions to generate thumbnails, medium sized images, rotate 	images, prompts user, writes index.html 
input_prompt.c
	borrows input_string to handle user input
input_prompt.h

assignment:
	concurrency: main is handling input from user on current display image, there 		is another process forked to handle thumbnail creation of next image 		which forks to another process to handle medium version creation of			next image
	coordination: cannot display image until at least thumbnail is generated, 			thus before entering loop I generate first thumbnail (and medium 			version), and every other display already has thumbnail ready to go 			due to concurrency handling user input
		similarly, cannot rotate current image until user input so those exec 		calls are forked after receiving string_input
