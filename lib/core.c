
int strlen( char *s)
{
  int  len;
  for ( len = 0;  *s != '\0';  len += 1, s += 1);
  return len;
}

