Likelihood Model for Symbol Template Matching

<<<<<<< HEAD
<<<<<<< HEAD
1, Given a binary image I and a symbol model M, we want to decide at any location (i,j) of I, whether there's a probable symbol candidate via data likelihood:
=======
1, Given a binary image and a symbol model, we want to determine at any location (i,j), whether there's a probable symbol candidate, by checking the data likelihood:
>>>>>>> 951f2aa... add symbol detector model explaination
=======
1, Given a binary image I and a symbol model M, we want to decide at any location (i,j) of I, whether there's a probable symbol candidate via data likelihood:
>>>>>>> a136b52... add symbol detector model explaination

P(I(i,j)|M) = P(p(i,j)|M)*P(p(i+1,j)|M)*...*P(p(i+n,j)|M)
            *P(p(i,j+1)|M)*P(p(i+1,j+1)|M)*...*P(p(i+n,j+1)|M)
            *...
            *P(p(i,j+m)|M)*P(p(i+1,j+m)|M)*...*P(p(i+n,j+m)|M)

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
which assumes each pixel in the region of interest is conditionally independent given the model M. The higher this likelihood is, the more possible there is such a symbol.

2, Training of M.
Given a set of symbol instances I(training dataset), we want to determine what's the probability distribution for each location (x,y) of model M. Since the observation is binary (only has two possible values: 0 or 1), the probability of M(x,y) can be easily estimated through Maximal-Likelihood-Estimation (MLE) from all the positive instances as follows:
=======
which assumes each pixel in the region of interest is conditionally independent given the model M.

2, Training of M.
Given a set of symbol instances (training dataset), we want to determine what's the probability distribution for each location (x,y) of model M. Since the observation is binary (only has two possible values: 0 or 1), so the probability of M(x,y) can be easily estimated through Maximal-Likelihood-Estimation (MLE) from all the positive instances:
>>>>>>> 951f2aa... add symbol detector model explaination
=======
which assumes each pixel in the region of interest is conditionally independent given the model M. The higher this likelihood is the more possible there is a such symbol.
=======
which assumes each pixel in the region of interest is conditionally independent given the model M. The higher this likelihood is, the more possible there is such a symbol.
>>>>>>> 28df13d... update masterscore omr

2, Training of M.
Given a set of symbol instances I(training dataset), we want to determine what's the probability distribution for each location (x,y) of model M. Since the observation is binary (only has two possible values: 0 or 1), the probability of M(x,y) can be easily estimated through Maximal-Likelihood-Estimation (MLE) from all the positive instances as follows:
>>>>>>> a136b52... add symbol detector model explaination

P(black | M(x,y)) = #(black at I(x,y))/(#(black at I(x,y)) + #(white at I(x,y))), for all I in the training set
P(white | M(x,y)) = 1 - P(black | M(x,y))

<<<<<<< HEAD
<<<<<<< HEAD
3, Thresholding for candidate detection
=======
3, Thresholding to detect candidates
>>>>>>> 951f2aa... add symbol detector model explaination
=======
3, Thresholding for candidate detection
>>>>>>> a136b52... add symbol detector model explaination
For test image I, we obtain the background model B by counting the number of all black's and white's on the image.

P(black | B) = #(black on I)/#(pixels on I)
P(white | B) = 1 - P(black | B)

<<<<<<< HEAD
<<<<<<< HEAD
Then the candidate scoring function can be written as

S(I(i,j)) = log(P(I(i,j)|M) / P(I(i,j)|B)) = Sum( log(P(p(i+ii,j+jj)|M)) - log(P(p(i+ii,j+jj)|B))), where p(i+ii,j+jj) is the pixel value (0 or 1) at position (i+ii, j+jj).
If S(I(i,j)) is larger than 0, we say at position (i,j) there's a symbols candidate. This threshold 0 is automatically determined by our background model, but can also be tuned according to our request (e.g. for higher precision than recall the threshold should be larger).
=======
So the candidate scoring function can be written as

S(I(i,j)) = log(P(I(i,j)|M) / P(I(i,j)|B)) = Sum( log(P(p(i+ii,j+jj)|M)) - log(P(p(i+ii,j+jj)|B))), where p(i+ii,j+jj) is the pixel value (0 or 1) at position (i+ii, j+jj).
If S(I(i,j)) is larger than 0, we say at position (i,j) there's a symbols candidate. This threshold 0 directly comes from background model, but also can be tuned according to our request (e.g. higher precision than recall then the threshold should be larger).
>>>>>>> 951f2aa... add symbol detector model explaination
=======
Then the candidate scoring function can be written as

S(I(i,j)) = log(P(I(i,j)|M) / P(I(i,j)|B)) = Sum( log(P(p(i+ii,j+jj)|M)) - log(P(p(i+ii,j+jj)|B))), where p(i+ii,j+jj) is the pixel value (0 or 1) at position (i+ii, j+jj).
If S(I(i,j)) is larger than 0, we say at position (i,j) there's a symbols candidate. This threshold 0 is automatically determined by our background model, but can also be tuned according to our request (e.g. for higher precision than recall the threshold should be larger).
<<<<<<< HEAD
>>>>>>> a136b52... add symbol detector model explaination
=======


Graphical Model for System Identification

1, Barline detection can be easily treated as finding vertical edges, but the results were unreliable since the "detection" process is sensitive to noise such as stems or lines in the text. The other problem of simply relying on detection is that we can hardly interpret the structure of systems based on the detected results. The solution to system identification is to use a graphical model to represent the system structure and encode useful distance and non-overlapping constraints, with which we can determine the grouping of staves into systems and the location of barlines in each system at the same time (simultaneously estimate both).

2, Suppose we have n staves (n-1 gaps), then there'll be 2^(n-1) ways of system grouping if taking each gap as a binary switch connecting or not connecting the adjacent staves. In each system (staff group), the barline positions will be commonly shared (a very strong and useful constriant!!). We use a nested dynamic programming to solve this problem. The optimal hypothesis (how the staves are grouped together) until k-th stave yielding the max score h(k) = max(h(i) + system(i+1, ..., k)), based on previous optimal hypotheses h(i), i= 1,2,...,k-1.

In each hypothesized system(i,...,j) from i-th to j-th staves, we recognize shared barlines from left to right by finding the best scoring configuration
b_opt = max(b(k1) + b(k2) + ... + b(kn)) supposing each horizontal location contains a bar or just background, where b(.) is the scoring function of barline in that column. We can also put negative constraints (note stem) into this bar line recognition process.

>>>>>>> 28df13d... update masterscore omr
