BEGIN {
    count = 0;
}

{
    count++;
if(count == 12)
{
    count = 0;
    printf "%s\n", $0;
}
else
{
    printf "%s,", $0;
}
}
