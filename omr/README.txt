Likelihood Model for Music Symbol Template Matching

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
If S(I(i,j)) is larger than 0, we say at position (i,j) there's a symbols candidate. This threshold 0 is automatically determined by our background model, but can also be tuned according to our request (e.g. for higher precision than recall the threshold should be larger).
