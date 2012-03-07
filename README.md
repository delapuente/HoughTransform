# Hough Transform 

In artificial vision, the [hough transform](http://http://en.wikipedia.org/wiki/Hough_transform) is a technique to discover borders by transforming border points into parametric space where they become lines. In the other hand, in this parametric space, border shapes (such as straight lines or curves) are represented by points. Those points where a lot of lines intersect are likely to be actual borders.

## This implementation

This implementation shows the simplest variation of hough transform by looking for straight lines. The parametric space choosen represents lines in polar coordinates to avoid representation issues.

The code is the result of an academic project to look for cultive lines in high resolution images. To achieve this goal I used binaryzed images where white color indicate information. Some improvements this implementation includes are:

 * **Light in memory** by using down-scaled versions of the images. Despite black (non-relevant) points are skipped, in order to look for straight lines, not much points are required so the input image is down-scalled (manually) before processed.
 * **Precached** `sin` and `cos` values.
 * **Integer arithmetic** when possible.
 * `O(n)` complexity by **joining tranformation into parametric space and clustering**.

## Possible improvements

 * Parametric adjustement is neccesary
 * Filtering can be added to transformation stage in order to discard impossible values earlier.
 * Other types of *down-scaling* can be tested to discard more non-relevant information.

## Building Hough Transform

You can build the sources by importing the project from Eclipse or you can use `make all` from `Release` and `Debug` folders as well.

Hough Transform was created using **[Eclipse 3.7.1](http://www.eclipse.org/)** 

## Documentation

There is no documentation and it is not intended to be. The code is just an example of implementation and it is clear enougth to be self-explanatory.

You can find more info in [my hought transformation space](http://unoyunodiez.com/houghtransform/).

## Version history

### 20120307 (1.0)

 * Initial public release

## License

See LICENSE.txt to check license terms and conditions.

