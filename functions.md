# Functions

AltF
- Get `current_alternative` for ColorSelect<>, TrSelect<> or IntSelect<>
- No template arguments
- Returns Int

SyncAltToVarianceF
- Synchronize Alt and Variance (so if one changes, so does the other)
- no template arguments
- Returns 0 (a dummy return, this is a utility function)

SyncAltToVarianceL
- A wrapper for the function as a layer
- returns transparent layer

BatteryLevel
- Get battery level 
- No template arguments
- Returns (int16_t) starting from 0 

BladeAngleX
- Get angle of blade from 0 (down) and int16_t lim (up) 
- Args:
    * Min: Int function from 0 to int16_t lim to specify cutoff (0)
    * Max: specify max cutoff (int16_t lim)
- Min and max essentially scale the range of angles to a specific section. 
    Setting Min to 1/2 int16_t lim will result in only the top angles being reported,
    but the range is always 0 to int16_t lim (So anything below the min is 0, and vice versa if using max)
- Returns int function

BladeAngle
- BladeAngleX but wrapped to take raw ints instead of INT class

BlastF
- Creates a wave starting at blast location
- Args:
    * FADEOUT_MS: int specifying fade time (200)
    * WAVE_SIZE: int specifying blast "wave" width (in pixels?) (100)
    * WAVE_MS: int speed of waves (400)
    * EFFECT: the type of effect to trigger the blast (EFFECT_BLAST)
- Returns a function (INT) to be Mix<>'d or AlphaL<>'d.

BlastFadeoutF
- Like BlastF, but without any wave effect, same for all LEDs
- Args:
    * FADEOUT_MS: int specifying fade time (250)
    * EFFECT: type of effect to trigger the blast (EFFECT_BLAST)
- Returns int function (int16_t)

OriginalBlastF
    Usage: OriginalBlastF<EFFECT>
    EFFECT: a BladeEffectType (defaults to EFFECT_BLAST)
    return value: FUNCTION
    Original blast function. Normally returns zero, but
    returns up to 32768 when the selected effect occurs.

BlinkingF
    Usage: BlinkingF<BLINK_MILLIS, BLINK_PROMILLE>
    BLINK_MILLIS: a number
    BLINK_PROMILLE: a number, defaults to 500
    return value: FUNCTION
    Switches between 0 and 32768
    A full cycle from 0 to 328768 and back again takes BLINK_MILLIS milliseconds.
    If BLINK_PROMILLE is 500, we select A for the first half and B for the
    second half. If BLINK_PROMILLE is smaller, we get less A and more B.
    If BLINK_PROMILLE is 0, we get all 0.
    If BLINK_PROMILLE is 1000 we get all 32768.

BrownNoiseF
    Usage: BrownNoiseF<GRADE>
    return value: FUNCTION
    Returns a value between 0 and 32768 with nearby pixels being similar.
    GRADE controls how similar nearby pixels are.
    
SlowNoise
   Usage: SlowNoise<SPEED>                                           
   return value: FUNCTION
   Returns a value between 0 and 32768 which changes randomly up and
   down over time. All pixels gets the same value.
   SPEED controls how quickly the value changes.

Bump
   Usage: Bump<BUMP_POSITION, BUMP_WIDTH_FRACTION>                         
   Returns different values for each LED, forming a bump shape.
   If BUMP_POSITION is 0, bump will be at the hilt.
   If BUMP_POSITION is 32768, the bump will be at the tip.
   If BUMP_WIDTH_FRACTION is 1, bump will be extremely narrow.
   If BUMP_WIDTH_FRACTION is 32768, it will fill up most/all of the blade.
   BUMP_POSITION, BUMP_WIDTH_FRACTION: INTEGER

HumpFlickerF (HumpFlickerFX)
   Usage: HumpFlickerFX<FUNCTION>                      
   or: HumpFlickerF<N>
   FUNCTION: FUNCTION
   N: NUMBER
   return value: INTEGER
   Creates hump shapes that randomize over the blade.
   The returned INTEGER is the size of the humps.
   Large values can give the blade a shimmering look, 
   while small values look more like speckles.

CenterDistF
   Usage: Remap<CenterDistF<CENTER>,COLOR>    
   Distributes led COLOR from CENTER
   CENTER : FUNCTION (defaults to Int<16384>)
   *RNOTE: This seems to just get a value for a curve from a given center?
        uint16_t max val is divided 

ChangeSlowly
   Usage: ChangeSlowly<F, SPEED>                      
   Changes F by no more than SPEED values per second.
   F, SPEED: FUNCTION
   return value: FUNCTION (F after change), same for all LEDs

CircularSectionF
   Usage: CircularSectionF<POSITION, FRACTION>                                       
   POSITION: FUNCTION position on the circle or blade, 0-32768
   FRACTION: FUNCTION how much of the blade to light up, 0 = none, 32768 = all of it
   return value: FUNCTION
   Returns 32768 for LEDs near the position with wrap-around.
   Could be used with MarbleF<> for a marble effect, or with
   Saw<> for a spinning/colorcycle type effect.
   Example: If POSITION = 0 and FRACTION = 16384, then this function
   will return 32768 for the first 25% and the last 25% of the blade
   and 0 for the rest of the LEDs.

ClampF (ClampFX)
   Usage: ClampF<F, MIN, MAX>            
   Or:    ClampFX<F, MINCLASS, MAXCLASS>
   Clamps value between MIN and MAX
   F, MIN, MAX: INTEGER
   MINCLASS, MAXCLASS: FUNCTION
   return value: INTEGER

ClashImpactF (ClashImpactFX)
   Usage: ClashImpactFX<MIN, MAX>                                                         
   MIN is minimum value Clash is detected (recommended range 0 ~ 500, default is 200)
   MAX is maximum impact to return 32768 (recommended range 1000 ~ 1600, default is 1600)
   Returns 0-32768 based on impact strength of clash
   returned value: INTEGER

Divide
   Usage: Divide<F, V>                                                                                  
   Divide F by V
   If V = 0, returns 0
   F, V: FUNCTION, 
   return value: FUNCTION
   Please note that Divide<> isn't an exact inverse of Mult<> because mult uses fixed-point mathematics
   (it divides the result by 32768) while Divide<> doesn't, it just returns F / V

EffectPulse
   Usage: EffectPulse<EFFECT>                                
   EFFECT: BladeEffectType
   Returns 32768 once for each time the given effect occurs.

LockupPulseF
   Usage: LockupPulseF<LOCKUP_TYPE>                          
   LOCKUP_TYPE: a SaberBase::LockupType
   Returns 32768 once for each time the given lockup occurs.

IncrementWithReset
   Usage: IncrementWithReset<PULSE, RESET_PULSE, MAX, I>              
   PULSE: FUNCTION (pulse type) 
   RESET_PULSEE: FUNCTION (pulse type) defaults to Int<0> (no reset)
   MAX, I: FUNCTION
   Starts at zero, increments by I each time the PULSE occurse.
   If it reaches MAX it stays there.
   Resets back to zero when RESET_PULSE occurs.

EffectIncrementF
   Usage: EffectIncrementF<EFFECT, MAX, I>                                            
   Increases by value I (up to MAX) each time EFFECT is triggered
   If current value + I = MAX, it returns 0.
   If adding I exceeds MAX, the function returns 0 + any remainder in excesss of MAX 
   I, MAX = numbers
   return value: INTEGER

EffectPosition
   Usage: EffectPosition<>                                                                      
   Or: EffectPosition<EFFECT>
   EFFECT: effect type
   return value: INTEGER
                                                                                                
   EffectPosition returns the position of a particular effect. 0 = base, 32768 = tip.
   For now, this location is random, but may be set explicitly in the future.
   When used as EffectPosition<> inside a TransitionEffectL whose EFFECT is already specified, 
   then it will automatically use the right effect.

HoldPeakF
   Usage: HoldPeakF<F, HOLD_MILLIS, SPEED>       
   Holds Peak value of F for HOLD_MILLIS.
   then transitions down over SPEED to current F
   F, HOLD_MILLIS and SPEED: FUNCTION
   return value: FUNCTION, same for all LEDs

Ifon
   Usage: Ifon<A, B>
   Returns A if saber is on, B otherwise.
   A, B: INTEGER
   return value: INTEGER 

InOutFunc
   InOutFunc<OUT_MILLIS, IN_MILLIS>                                  
   IN_MILLIS, OUT_MILLIS: a number
   RETURN VALUE: FUNCTION
   0 when off, 32768 when on, takes OUT_MILLIS to go from 0 to 32768
   takes IN_MILLIS to go from 32768 to 0.

InOutFuncX
    InOutFunc, but takes Ints instead of raw ints

InOutHelperF
    Dunno, no docs

InOutFuncTD
    Dunno, no docs

# Left off on ifon.h


