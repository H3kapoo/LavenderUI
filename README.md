# Layout Calculator for GUI

This will give a rough ideea of the requirements and capabilities o
## User Requirements

1. User has the abilty to set
    1. Scale (Pixel/Relative/Fit/Fill)
    2. Position (if not auto computed)
    3. Wrapping
    4. Layout type (Horizontal/Vertical/Grid)
    5. Alignment of objects inside the element
    6. Spacing inside the element
    7. Margins
    8. Padding
    9. Border
    10. Border radius
    11. Min & Max scale
    12. Custom Z index (if `2. Position` is not auto computed)
2. User has the ability to add or remove child elements on demand to and from GUI elements that support this operation.
3. User set scale actually represents the final element's scale which includes the margins the object. The whole element's area is used to position other elements of the same parent.
4. The inside content area of the element is shrunk by it's padding and borders. The content area is used to place child elements inside of the element.
5. Children of elements outside of the user's view shall not be rendered nor calculated as that would be pointless.
6. Scrollbars will always sit an ``offset`` amount of Z indices higher than the container they scroll. Stacking child elements inside the element
   can increase the Z index above the scrollbar's index and have them render on top of it, which is not desireable. 
7. [GUI elements that support scroll bars](#objects-that-support-scrolling) have the ability to interact with the user via common manipulation techniques:
   1. Mouse scrolling up and down changes the sub-view the user can see from the bigger layout.
   2. Clicking outside the scrollbar's knob will scroll the view such that the mouse position is always in the middle of the knob's
        scale. [See Exception 1](#exceptions)
   3. Clicking and dragging the mouse up and down has the effect of changing the sub-view by the same amount the user has dragged.
        The mouse will not jump up and down in order to be located at the center of the knob's scale as in point 2. An offset from
        the knob's center and the mouse position shall be used instead.
8. [GUI elements that support recycling](#objects-that-support-recycling) shall efficiently use memory such that only the the viewable area is populated by child elements and the
   rest of the items reside in memory ready to be swapped in and out as needed when scrolling or resizing the parent element.

## Exceptions
1. If knob position cannot be achieved (i.e. no more space to move the knob up or down), the closest distance to that ideal shall be used.


## General box model and definitions
### Box model
The box model used is comprised of the following, in order from outside to inside:
   1. Margins
   2. Borders
   3. Padding
   4. Actual content area


> **Note:** When the user sets the scale to be `(100, 100)` but he also has a margin of `(2, 2, 2, 2)` then this means that the actual, whole scale of the element is `user scale + margins` aka `(100 + 2ML + 2MR, 100 + 2MT + 2MB) = (104, 104)`

> **Note:** When the user sets the scale to be `(100, 100)` but he also has a padding of `(2, 2, 2, 2)` and a border size of `(4, 4, 4, 4)` then this means that the actual content area left is `user scale - padding - border size` aka `(100 - 2PL - 2PR - 4BL - 4BR, 100 - 2PT - 2PB - 4BT - 4BB) = (88, 88)`

### Definitions
> `user scale` Refers to the user set preferred whole scale which includes margins. <br/> => user scale = margins + borders + padding + content area

> `user pos` Refers to the user set preferred element start position which includes margins. <br/> => user pos = content area - borders - padding - margins

> `computed scale` Refers to the scale that will be sent to the GPU for rendering. It does not contain the margins. This is what the layout calculator actually computes. <br/> => computed scale = borders + padding + content area

> `computed pos` Refers to the pos that the mesh that will be sent to GPU for rendering starts from. It does not contain the margins. This is what the layout calculator also computes. <br/> => computed pos = content area - borders - padding

> `content scale` Refers to the scale of the available content area. <br/> => content scale = user scale - borders - padding - margins.

> `PX scale` Means the scale on that axis is represented by fixed pixel values.

> `REL scale` Means the scale of the element is relative to the content size of it's parent (0 to 1 value).

> `FIT scale` Means the scale of the element will try to fit around the child elements as best as possible just like a "gift wrap".

> `FILL scale` Means the element will occupy an equal part with other fill scale elements inside the parent's content scale. The equal part is calculated with the remaining space after all the other scale types are dealt with.

## Calculating generic element scale (horizontal/vertical layout type)

By calculating the element's scale we mean the `computed scale` (renderable area) of that element. <br/>
The calculator will receive the element `(parent)` who's child elements need to be calculated. <br/>

> **Warning:** If the parent element is an element that can host a scrollbar then scale calculations for it shall be skipped (it will be calculated separately).

The following pseudocode encapsulates both the X and Y axis in the same algorithm. When implementing be sure to do calculations per each axis. Any futher optimizations are left for the implementer:

```Python
   runningTotal = 0
   fillElementCount = 0
   for element in parent.elements:
      if element.isPX:
         element.computedScale.x = element.userScale - element.margin
      elif element.isREL:
         element.computedScale = parent.contentScale * element.userScale - element.margin
      elif element.isFIT:
         element.computedScale = getFitScale(element) - element.margin
      elif element.isFILL:
         fillElementCount += 1

      runningTotal += element.computedScale

   if not fillElementCount:
      return

   equalFillPart = (parent.contentScale - runningTotal) / fillElementCount
   for element in parent.elements:
      if not element.isFILL:
         continue

      element.computedScale = equalFillPart - element.margin
```

<br/>

Computing the fitting scale of around the parent's elements is a bit tricky as they can be `FIT` themselves.
The algorithm returns the scale that thighly best fits all the child elements and takes into account parent's margins, border and padding. From the caller perspective it looks like it returns a userScale-like value as if the parent had only one child element.

> **Warning:** Scrollbars must be skipped here as well in case there are any.

> **Warning:** If the parent is FIT on one axis then the leaf child elements on that axis must be of scale type PX. Otherwise it is impossible to fit around them.

```Python
   def getFitScale(parent):
      if parent.elements.size == 0:
         return # ERROR, No elements to fit around

      fitScale = {0, 0}
      for element in parent.elements:
         computedValue = {0, 0}
         if parent.isXAxisFIT:
            if element.isXAxisPX:
               computedValue.x = element.userScale.x
            elif element.isXAxisFIT:
               computedValue.x = getFitScale(element).x
            else: # ERROR

            fitScale.x = max(fitScale.x, computedValue.x) if parent.isVertical else fitScale.x + computedValue.x

         if parent.isYAxisFITX:
            if element.isYAxisPX:
               computedValue.y = element.userScale.y
            elif element.isYAxisFIT:
               computedValue.y = getFitScale(element).y
            else: # ERROR

            fitScale.y = max(fitScale.y, computedValue.y) if parent.isHorizontal else fitScale.y + computedValue.y

      fitScale += parent.border + parent.padding + parent.margin
      return fitScale
```

## Calculating generic element position (horizontal/vertical layout type)

By calculating the element's position we mean the `computed pos` (start of renderable area) of that element. <br/>
The calculator will receive the element `(parent)` who's child elements need to be calculated. <br/>

> **Warning:** If the parent element is an element that can host a scrollbar then scale calculations for it shall be skipped (it will be calculated separately).

## Objects that support scrolling
1. UIPane
2. UITreeView

## Objects that support recycling
1. UITreeView
