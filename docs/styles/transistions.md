*Reformatted from transitions.cpp by ChatGPT*
 
## Exponential Time Bend (Number)
*[BendTimePowX]*

### Description
This function calculates an exponential time bend based on the provided time and power parameters.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the time in milliseconds.
- **Power**
  - Type: Number [Func]
  - Description: Specifies the power for the exponential calculation.

---

## Exponential Time Bend (Number)
*[BendTimePow]*

### Description
This function calculates an exponential time bend based on the provided time and power parameters.

### Arguments:
- **Time (ms)**
  - Type: Number
  - Description: Specifies the time in milliseconds.
- **Power**
  - Type: Number
  - Description: Specifies the power for the exponential calculation.

---

## Inverse Exponential Time Bend (Number)
*[BendTimePowInvX]*

### Description
This function calculates an inverse exponential time bend based on the provided time and power parameters.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the time in milliseconds.
- **Power**
  - Type: Number [Func]
  - Description: Specifies the power for the inverse exponential calculation.

---

## Inverse Exponential Time Bend (Number)
*[BendTimePowInv]*

### Description
This function calculates an inverse exponential time bend based on the provided time and power parameters.

### Arguments:
- **Time (ms)**
  - Type: Number
  - Description: Specifies the time in milliseconds.
- **Power**
  - Type: Number
  - Description: Specifies the power for the inverse exponential calculation.

---

## Reverse Time (Number)
*[ReverseTimeX]*

### Description
This function reverses the time.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the time in milliseconds.

---

## Reverse Time (Number)
*[ReverseTime]*

### Description
This function reverses the time.

### Arguments:
- **Time (ms)**
  - Type: Number
  - Description: Specifies the time in milliseconds.

---

## Blink (Transition)
*[TrBlinkX]*

### Description
Creates a blinking effect for a specified number of times.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of each blink in milliseconds.
- **Number of Blinks**
  - Type: Number
  - Description: Specifies the number of blinks.
- **Distribution**
  - Type: Number [Func]
  - Description: Specifies the distribution of the blink effect. Default value is 16384 if not provided.

---

## Blink (Transition)
*[TrBlink]*

### Description
Creates a blinking effect for a specified number of times.

### Arguments:
- **Time (ms)**
  - Type: Number
  - Description: Specifies the duration of each blink in milliseconds.
- **Number of Blinks**
  - Type: Number
  - Description: Specifies the number of blinks.
- **Distribution**
  - Type: Number
  - Description: Specifies the distribution of the blink effect. Default value is 16384 if not provided.

---

## Smooth Fade (Transition)
*[TrSmoothFadeX]*

### Description
Creates a smooth fading effect between two colors over a specified duration.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the fade effect in milliseconds.
---

## Smooth Fade (Transition)
*[TrSmoothFade]*

### Description
Creates a smooth fading effect between two colors over a specified duration.

### Arguments:
- **Time (ms)**
  - Type: Number
  - Description: Specifies the duration of the fade effect in milliseconds.

---

## Instant (Transition)
*[TrInstant]*

### Description
Instantly transitions between colors.

### Arguments:
None

---

## Combine (Transition)
*[TrJoin]*

### Description
Combines multiple transitions to run in parallel.

### Arguments:
- **Transition 1**
  - Type: Transition [Func]
  - Description: Specifies the first transition.
- **Transition 2**
  - Type: Transition [Func]
  - Description: Specifies the second transition.
- **Transition #**
  - Type: Transition [Func]
  - Description: Additional transitions to combine.

---

## Reverse Combine (Transition)
*[TrJoinR]*

### Description
Combines multiple transitions to run in parallel, chaining to the right.

### Arguments:
- **Transition 1**
  - Type: Transition [Func]
  - Description: Specifies the first transition.
- **Transition 2**
  - Type: Transition [Func]
  - Description: Specifies the second transition.
- **Transition #**
  - Type: Transition [Func]
  - Description: Additional transitions to combine.

---

## Loop Forever (Transition)
*[TrLoop]*

### Description
Runs the specified transition in a loop forever.

### Arguments:
- **Transition**
  - Type: Transition [Func]
  - Description: Specifies the transition to loop.

---

## Loop (Transition)
*[TrLoopNX]*

### Description
Runs the specified transition a specified number of times.

### Arguments:
- **Number of Loops**
  - Type: Number [Func]
  - Description: Specifies the number of times to loop the transition.
- **Transition**
  - Type: Transition [Func]
  - Description: Specifies the transition to loop.

---

## Loop Until (Transition)
*[TrLoopUntil]*

### Description
Runs the specified transition until a pulse occurs.

### Arguments:
- **End Pulse**
  - Type: Number [Func]
  - Description: Specifies the pulse to trigger the end of the loop.
- **Transition**
  - Type: Transition [Func]
  - Description: Specifies the transition to loop.
- **End Transition**
  - Type: Transition [Func]
  - Description: Specifies the transition to use when the loop ends.

---

## Random (Transition)
*[TrRandom]*

### Description
Selects a random transition from the provided list each time.

### Arguments:
- **Transition 1**
  - Type: Transition [Func]
  - Description: Specifies the first transition.
- **Transition #**
  - Type: Transition [Func]
  - Description: Additional transitions to choose from.

---

## Selection (Transition)
*[TrSelect]*

### Description
Selects a transition from the provided list based on the selection number.

### Arguments:
- **Selection Number**
  - Type: Number [Func]
  - Description: Specifies the selection number to choose the transition.
- **Transition 1**
  - Type: Transition [Func]
  - Description: Specifies the first transition.
- **Transition #**
  - Type: Transition [Func]
  - Description: Additional transitions to choose from.

---

## Sequence (Transition)
*[TrSequence]*

### Description
Selects transitions sequentially from the provided list.

### Arguments:
- **Transition 1**
  - Type: Transition [Func]
  - Description: Specifies the first transition.
- **Transition #**
  - Type: Transition [Func]
  - Description: Additional transitions to select sequentially.

---  

## Wave (Transition)
*[TrWaveX]*

### Description
Creates a wave effect starting from a specified point on the blade.

### Arguments:
- **Color**
  - Type: Color
  - Description: Specifies the color of the wave.
- **Fadeout Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the fadeout effect in milliseconds.
- **Size**
  - Type: Number [Func]
  - Description: Specifies the size of the wave.
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the wave effect in milliseconds.
- **Position**
  - Type: Number [Func]
  - Description: Specifies the starting position of the wave on the blade.

---

## Spark (Transition)
*[TrSparkX]*

### Description
Generates a spark effect traveling along the length of the blade.

### Arguments:
- **Color**
  - Type: Color
  - Description: Specifies the color of the spark.
- **Size**
  - Type: Number [Func]
  - Description: Specifies the size of the spark.
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the spark effect in milliseconds.
- **Position**
  - Type: Number [Func]
  - Description: Specifies the starting position of the spark on the blade.

---

## Wipe (Transition)
*[TrWipeX]*

### Description
Creates a wipe effect from one color to another.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the wipe effect in milliseconds.

---

## Wipe In (Transition)
*[TrWipeInX]*

### Description
Creates a wipe effect from one color to another, starting from the tip of the blade.

### Arguments:
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the wipe effect in milliseconds.

---

## Wipe With Tip Spark (Transition)
*[TrWipeSparkTipX]*

### Description
Creates a wipe effect from one color to another with a spark at the leading edge.

### Arguments:
- **Spark Color**
  - Type: Color
  - Description: Specifies the color of the spark.
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the wipe effect in milliseconds.
- **Spark Size**
  - Type: Number [Func]
  - Description: Specifies the size of the spark.

---

## Wipe In With Tip Spark (Transition)
*[TrWipeInSparkTipX]*

### Description
Creates a wipe effect from one color to another with a spark at the leading edge, starting from the tip of the blade.

### Arguments:
- **Spark Color**
  - Type: Color
  - Description: Specifies the color of the spark.
- **Time (ms)**
  - Type: Number [Func]
  - Description: Specifies the duration of the wipe effect in milliseconds.
- **Spark Size**
  - Type: Number [Func]
  - Description: Specifies the size of the spark.