apt-get update
apt-get install build-essential checkinstall 
mkdir ~/tesseract
cd ~/tesseract
wget http://www.leptonica.org/source/leptonica-1.73.tar.gz
tar xf leptonica-1.73.tar.gz && rm -rf leptonica-1.73.tar.gz
cd leptonica-1.73
./configure
make && checkinstall --pkgname=libleptonica --pkgversion="1.73" --backup=no --deldoc=yes --fstrans=no --default
cd ~/tesseract
wget https://github.com/tesseract-ocr/tesseract/archive/3.04.01.tar.gz
tar xf 3.04.01.tar.gz && rm -rf 3.04.01.tar.gz
cd tesseract-3.04.01
./autogen.sh
./configure
make && checkinstall --pkgname=tesseract-ocr --pkgversion="3.04.01" --backup=no --deldoc=yes --fstrans=no --default && ldconfig
mkdir ~/tesseract/langs
cd ~/tesseract/langs
wget https://raw.githubusercontent.com/tesseract-ocr/tessdata/master/eng.traineddata
cp eng.traineddata /usr/local/share/tessdata/
echo -e "\ntesseract output:"
tesseract --version && tesseract --list-langs && cd ~ && rm -rf ~/tesseract
