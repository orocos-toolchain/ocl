inline unsigned short swapEndian (unsigned short theProblem)
{
  return (theProblem << 8) | (theProblem >> 8);
}
 
