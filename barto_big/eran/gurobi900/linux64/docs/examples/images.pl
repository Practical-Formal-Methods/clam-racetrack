# LaTeX2HTML 2019 (Released January 1, 2019)
# Associate images original text with physical files.


$key = q/scalebox{1.0}{includegraphics[width=60mm]{logo}};AAT/;
$cached_env_img{$key} = q|<IMG STYLE=""
 SRC="|."$dir".q|img1.svg"
 ALT="\scalebox{1.0}{\includegraphics[width=60mm]{logo}}">|; 

1;

