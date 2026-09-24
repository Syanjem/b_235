#include "algorithm.h"











// 冒泡排序
void bubble_sort(uint16_t arr[], uint8_t len) 
{
    for (uint8_t i = 0; i < len - 1; i++) 
	{
        for (uint8_t j = 0; j < len - 1 - i; j++) 
		{
            if (arr[j] > arr[j + 1]) 
			{
                uint16_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}











