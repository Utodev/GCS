long i86_ptr_to_long(unsigned offset, unsigned seg)
{
long result;

result=seg;
result <<= 4;
result+=offset;

return(result);
}

