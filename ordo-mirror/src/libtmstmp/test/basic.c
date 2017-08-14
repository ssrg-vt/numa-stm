#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <libtmstmp.h>

int main(int argc, char *argv[])
{
	printf("unordered time: %Lu\n", get_unordered_core_timestamp());
	printf("ordered time: %Lu\n", get_rdered_core_timestamp());
	printf("epoch: %Lu\n", get_global_epoch_timestamp());
	return 0;
}
