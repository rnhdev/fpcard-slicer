fpcard-slicer - An slicer for non-standard fingerprints cards
===========================
Is a application for to extract fingerprints of non-standard teen fingerprint card
# Building form sources
## Pre-requisites
### Linux
```sh
 $ [sudo] apt-get install build-essential autoconf libtool pkg-config libjpeg-dev libpng-dev
```
## Compile and run
### Linux
```sh
 $ git clone https://github.com/rnhdev/fpcard-slicer
 $ cd fpcard-slicer
 $ mkdir build 
 $ cd build
 $ cd cmake .. && make
 $ ./fpcard-slicer -s ../test -d ../test -q 10 -f jpg -o
 ```
## Limitations
Only support scanned images in 500dpi in jpeg or png format
## Example
![alt text](test/fcard-01.jpg "fingerprint card")
![alt text](test/fcard-01/01_binarized.jpg "scaling and binarized image")
![alt text](test/fcard-01/02_clip.jpg "clip")
![alt text](test/fcard-01/03_top.jpg "filtered top")
![alt text](test/fcard-01/04_bottom.jpg "filtered bottom")
![alt text](test/fcard-01/fp_0.jpg "fingerprint 1")
![alt text](test/fcard-01/fp_1.jpg "fingerprint 2")
![alt text](test/fcard-01/fp_2.jpg "fingerprint 3")
![alt text](test/fcard-01/fp_3.jpg "fingerprint 4")
![alt text](test/fcard-01/fp_4.jpg "fingerprint 5")
![alt text](test/fcard-01/fp_5.jpg "fingerprint 6")
![alt text](test/fcard-01/fp_6.jpg "fingerprint 7")
![alt text](test/fcard-01/fp_7.jpg "fingerprint 8")
![alt text](test/fcard-01/fp_8.jpg "fingerprint 9")
![alt text](test/fcard-01/fp_9.jpg "fingerprint 10")
