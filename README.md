### TODO():
	- add struct inside header based on encoding method [x]
	- modify secret header encoding and decoding methods [x]
	- finish encoding and decoding methods for LSB [x]
	- find the application flow []

### Application Flow

First 2 options:
- decode
- encode

1. Decode

- this should be done automatically, based on the encoding method saved in the header
- decode message

Based on encodingFormat:
	- IMAGE, display image
	- FILE, save file 
	- USER INPUT, 
  
Display Statistics:
	- time do decode
	- encoding method used 
	- encoding header

1. Encode:
   
	1. Choose type of secret format
	2. Choose secret
	3. Choose encoding method
		- (if possible check if the encoding method can work on that secret)
		- return an error if it cannot encode the message
	4. Show statistics
		- how much time it took
		- etc

2. Least Significant Bit (LSB) Replacement (Sequential)
3. LSB with Random Pixel Selection
4. Discrete Cosine Transform (DCT) Steganography (JPEG)
5. Frequency Domain Steganography (Wavelet Transform)
6. My method:
   
   - somethign with frequency analysis
   - choose more bits to hide the message based on the frequency of the respective color

https://dl.acm.org/doi/10.1145/3694965

- Adaptive
- Static

Three domains:

1. Spatial Domain (Least Significant Bit (LSB), Pixel Value Differencing (PVD) and Bit-Plane Complexity Segmentation (BPCS))
	- Hide messages directly in the intensity values of pixels
2. Transform Domain Approaches (Discrete Cosine Transform (DCT), Discrete Wavelet Transform (DWT), Integer Wavelet Transform (IWT) and Discrete Fourier Transform (DFT))
	- first transform the image and then hide the message
3. Deep Learning Approaches (Convolutional Neural Networks (CNN) and Generative Adversial Network (GAN))
	- use self-learned ways to embed a message

4. Spatial Domain

1.1 LSB
- simple LSB (the LSB pixel value is modified to embed the message)
- random LSB (user chooses the pixel number to embed the information)
- OPAP (Optimal Pixel Adjustment Process) (https://www.cs.nthu.edu.tw/~cchen/ISA5230/Ref/2004LSB-Cheng.pdf)

1.2 PVD (grayscale)
- the idea here is the human eye cannot perceive changes in edged areas so good as changes in smooth areas, therefore the difference between neighboring 
pixels
- zig-zag travsersal
- explain on image
- nice, can have a higher embedding capacity


https://scispace.com/pdf/pixel-value-differencing-a-steganographic-method-a-survey-ah9cz3lmhj.pdf
https://repository.root-me.org/St%C3%A9ganographie/EN%20-%20Pixel-Value%20Differencing%20Steganography%20-%20El-Alfy%20-%20Al-Sadi.pdf

1.3 BSP
- based on complexity of an image calculated using alpha (k/W-B changes)

https://citeseerx.ist.psu.edu/document?repid=rep1&type=pdf&doi=b701dfd8d262a64926ed05b6a29207fa3078116f
https://www.researchgate.net/publication/50346770_Review_Steganography_-_Bit_Plane_Complexity_Segmentation_BPCS_Technique

1.4 PVD (canny edge detection) (my method)