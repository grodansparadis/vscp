#!/usr/bin/perl
#
# X = -   Y = |    line fx,fy,tx,ty 

use GD;

$bredd=2400;
$hojd=350;
$givare=4;

$im = new GD::Image( $bredd, $hojd );

($white,$black,$gray,$green,$orange,$lightgray,$blue,$skyblue) =
  (
    $im->colorAllocate(255, 255, 255),
    $im->colorAllocate(0, 0, 0),
    $im->colorAllocate(196, 196, 196),
    $im->colorAllocate(0,255,0),
    $im->colorAllocate(255,0,255),
    $im->colorAllocate(220,220,220),
    $im->colorAllocate(0,0,255),
    $im->colorAllocate(100,100,255)
  );

$plotcolor[0] = $blue;
$plotcolor[1] = $orange;
$plotcolor[2] = $blue;
$plotcolor[3] = $orange;

$lines=($bredd-61)*$givare;

for ($a=0 ; $a<3 ; $a++)
{
  $xplotnum[$a]=31;
  $lastx[$a]=31;
  $lasty[$a]=0;
}


open(DOTTEMP, "tail -$lines /var/log/temperature|");
@dottemp = <DOTTEMP>;
close(DOTTEMP);

# Vertical lines for temperature.
for ($a=40 ; $a<=320 ; $a+=20)
{
  $im->line(30,$a,$bredd-30,$a,$gray);
}

# Temperature text to the right and to the left of lines.
for ($a=34 ; $a<=314 ; $a=$a+20)
{
  $im->string(gdSmallFont,4,$a,40-($a-34)/4,$skyblue);
  $im->string(gdSmallFont,$bredd-22,$a,40-($a-34)/4,$skyblue);
}

# The two black lines to the left and to the right
$im->line(30,10,30,330,$black);$im->line($bredd-30,10,$bredd-30,330,$black);

# Some text
$im->string(gdSmallFont,40,20,"Temperature plot",$black);
$im->string(gdSmallFont,180,20,"Inne",$plotcolor[0]);
$im->string(gdSmallFont,280,20,"Ute",$plotcolor[1]);

foreach (@dottemp)
{
  ($mo,$day,$date,$dead,$sensor,$dead,$centigrade,$dead,$dead) = split(" ", $_ );

  # Hack to make the line start from zero to the left
  if( $xplotnum[$sensor] eq '31' )
  {
    $lasty[$sensor]=$centigrade;
    $lastx[$sensor]=$xplotnum[$sensor];
  }

  $im->line( $lastx[ $sensor ], 
		200 - $lasty[ $sensor ] * 4, 
		$xplotnum[ $sensor ], 
		200 - $centigrade*4, 
		$plotcolor[ $sensor ] );

  $lasty[ $sensor ] = $centigrade;
  $lastx[ $sensor ] = $xplotnum[ $sensor ];
  $xplotnum[ $sensor ] = $xplotnum[ $sensor ] + 10; 

  # Text for time.
  ($dh,$dm,$ds) = split(":", $date );
  if( $dm eq '00' )
  {
     $im->string(gdSmallFont,$xplotnum[$sensor]-10,330,"$dh:$dm",$skyblue);
     $im->line($xplotnum[$sensor],40,$xplotnum[$sensor],330,$lightgray);
  }
}

# Text for latest temperature.
$im->string(gdSmallFont,210,20,$lasty[1],$plotcolor[0]);
$im->string(gdSmallFont,310,20,$lasty[0],$plotcolor[1]);
$im->string(gdSmallFont,420,20,"$mo $day -  $date",$black);

# Write to file
open( PLOT, ">/tmp/bjornplot.gif");
print PLOT $im->gif;
close(PLOT);























