
#Updates on Optical Music Recognition 

###Graphical Model for System Identification

1. We can simply use vertical line detection to find bar lines. But the detection performance is unreliable due to noise such as note stems or lines in text. The other problem of solely relying on line detection is that we can hardly interpret the structure of systems based on what has been detected. The solution to this problem is using a graphical model to represent the system structure and encode useful distance or non-overlapping constraints, with which we can determine the grouping of staves into systems and the location of barlines in each system at the same time (simultaneously estimate both).

2. Suppose we have n staves (n-1 gaps), then there'll be 2^(n-1) ways of grouping systems if taking each gap as a binary switch connecting or not connecting the adjacent staves. In each system (staff group), barline positions will be commonly shared (a very strong and useful constriant!). We can use a nested dynamic programming to solve this problem. The optimal hypothesis (how the staves are grouped together) until the k-th stave yielding the max score h(k) = max(h(i) + system(i+1, ..., k)), based on previous optimal hypotheses h(i), i= 1,2,...,k-1.
In each hypothesized system(i,...,j) from i-th to j-th staves, we recognize shared barlines from left to right by finding the best scoring configuration
b_opt = max(b(k1) + b(k2) + ... + b(kn)) supposing each horizontal location corresponds to a bar or just background, where b(.) is the scoring function for barline in that column. We can also incorporate negative constraints (clef, key sigs, time sigs, or note stem) into this bar line recognition process.


<!--Likelihood Model for Symbol Template Matching

1, Given a binary image I and a symbol model M, we want to decide at any location (i,j) of I, whether there's a probable symbol candidate via data likelihood:

P(I(i,j)|M) = P(p(i,j)|M)*P(p(i+1,j)|M)*...*P(p(i+n,j)|M)
            *P(p(i,j+1)|M)*P(p(i+1,j+1)|M)*...*P(p(i+n,j+1)|M)
            *...
            *P(p(i,j+m)|M)*P(p(i+1,j+m)|M)*...*P(p(i+n,j+m)|M)

which assumes each pixel in the region of interest is conditionally independent given the model M. The higher this likelihood is, the more possible there is such a symbol.

2, Training of M.
Given a set of symbol instances I(training dataset), we want to determine what's the probability distribution for each location (x,y) of model M. Since the observation is binary (only has two possible values: 0 or 1), the probability of M(x,y) can be easily estimated through Maximal-Likelihood-Estimation (MLE) from all the positive instances as follows:

P(black | M(x,y)) = #(black at I(x,y))/(#(black at I(x,y)) + #(white at I(x,y))), for all I in the training set
P(white | M(x,y)) = 1 - P(black | M(x,y))

3, Thresholding for candidate detection
For test image I, we obtain the background model B by counting the number of all black's and white's on the image.

P(black | B) = #(black on I)/#(pixels on I)
P(white | B) = 1 - P(black | B)

Then the candidate scoring function can be written as

S(I(i,j)) = log(P(I(i,j)|M) / P(I(i,j)|B)) = Sum( log(P(p(i+ii,j+jj)|M)) - log(P(p(i+ii,j+jj)|B))), where p(i+ii,j+jj) is the pixel value (0 or 1) at position (i+ii, j+jj).
If S(I(i,j)) is larger than 0, we say at position (i,j) there's a symbols candidate. This threshold 0 is automatically determined by our background model, but can also be tuned according to our request (e.g. for higher precision than recall the threshold should be larger).-->

###Demo
[Screenshots of some results](https://musescore.org/en/node/110306#comment-500796)

###Todo
* Add robust note detector to impose negative constraints for bar line identification. (see [here](https://github.com/musescore/MuseScore/blob/master/omr/omrpage.cpp#L292))
* Fix staff detection for vector graphs
* Optimize OMR performance and add clef/key recognitions
