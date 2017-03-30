import math, operator
from PIL import Image
def compare(file1, file2):
    image1 = Image.open(file1)
    image2 = Image.open(file2)
    h1 = image1.histogram()
    h2 = image2.histogram()
    rms = math.sqrt(reduce(operator.add,
                           map(lambda a,b: (a-b)**2, h1, h2))/len(h1))
    return rms

if __name__=='__main__':
    import sys
    if len(sys.argv) < 3:
        print "Usage: python picturediff.py <picture1> <picture2>"
        quit()
    file1, file2 = sys.argv[1:]
    print compare(file1, file2)
