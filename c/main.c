//
// Created by yateu on 10/02/2019.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "headers/utils.h"
#include "headers/main.h"
#include "headers/gifenc.h"
#include "headers/constants.h"
#include "headers/individual.h"
#include "headers/generation.h"

#ifdef _WIN32
#include <dir.h>
#else
#include <sys/stat.h>

#endif

int main(int argv, char ** argc)
{
	setbuf(stdout, 0);
	printf("Starting...\n");
	
	srand(time(NULL));

#ifdef _WIN32
	mkdir("output");
#else
	mkdir("output", 0777);
#endif
	
	int generate = 1;
	Generation * generation = GenerationCreate();
	for(int i = 0; i < 9; i++)
	{
		GenerationAppend(generation, IndividualCreate(ParametersCreateRandom()));
	}
	
	while(generate == 1)
	{
		printf("Generating generation...\n");
		
		GenerationGenerate(generation);
		char filename[1024];
		CreateHTML(filename, generation);
		char command[1024];
		sprintf(command, "open %s", filename);
		printf("Opening %s result: %d\n", filename, system(command));
		
		printf("Create new generation (1=yes, 0=no)? ");
		scanf("%d", &generate);
		
		if(generate == 1)
		{
			printf("Sequence to keep: ");
			int keep = 0;
			scanf("%d", &keep);
			
			int parentsCount = 0;
			Individual ** parents = malloc(sizeof(Individual *) * generation->populationSize);
			for(int individualIndex = generation->populationSize - 1; individualIndex >= 0; individualIndex--)
			{
				if(keep % 2 == 1)
				{
					parents[parentsCount] = generation->individuals[individualIndex];
					parentsCount++;
				}
				keep /= 10;
			}
			
			Generation * newGeneration = GenerationCreate();
			for(int parentIndex = 0; parentIndex < parentsCount; ++parentIndex)
			{
				GenerationAppend(newGeneration, IndividualCopy(parents[parentIndex]));
			}
			
			printf("Generating %d child(s)\n", generation->populationSize - parentsCount);
			for(int childIndex = 0; childIndex < generation->populationSize - parentsCount; childIndex++)
			{
				if(parentsCount == 0)
				{
					GenerationAppend(newGeneration, IndividualCreate(ParametersCreateRandom()));
				}
				else if(parentsCount == 1)
				{
					Individual * child = IndividualCopy(parents[0]);
					mutate(child, 1);
					GenerationAppend(newGeneration, child);
				}
				else
				{
					int parent1Index = (int) getRandom(0, parentsCount);
					int parent2Index = (int) getRandom(0, parentsCount - 1);
					if(parent1Index == parent2Index)
					{
						parent2Index++;
					}
					Individual * child = makeBaby(parents[parent1Index], parents[parent2Index]);
					if(getRandom(0, 1) < 0.1)
					{
						mutate(child, 0);
					}
					GenerationAppend(newGeneration, child);
				}
			}
			
			GenerationDestroy(generation);
			generation = newGeneration;
		}
	}
	GenerationDestroy(generation);
	
	return 0;
}

void CreateHTML(char * filename, Generation * pGeneration)
{
	sprintf(filename, "output/output-%lu-.html", getMicroTime());
	FILE * htmlFile = fopen(filename, "w");
	
	fprintf(htmlFile, "<html><body><table>");
	
	for(int i = 0; i < pGeneration->populationSize; i++)
	{
		Individual * individual = pGeneration->individuals[i];
		if(i % 2 == 0)
		{
			fprintf(htmlFile, "<tr>");
		}
		
		fprintf(htmlFile, "<td>%d - %s</td><td><img src=\"../%s\"/></td>", i + 1, individual->outputfile, individual->outputfile);
		
		if(i % 2 != 0 || i == pGeneration->populationSize - 1)
		{
			fprintf(htmlFile, "</tr>");
		}
	}
	
	fprintf(htmlFile, "</table></body></html>");
	
	fclose(htmlFile);
}

int keepParent(int individualIndex, Individual * individual)
{
	int keep = 0;
	printf("Keep %d - %s as a parent (1=yes, 0=no)? ", individualIndex + 1, individual->outputfile);
	scanf("%d", &keep);
	return keep;
}

void mutate(Individual * individual, int force)
{
	if(force == 1 || getRandom(0, 1) < 0.01)
	{
		individual->parameters->reaction_rate_a = MAX(0, MIN(1, individual->parameters->reaction_rate_a + getRandom(-0.05, 0.05)));
	}
	if(force == 1 || getRandom(0, 1) < 0.01)
	{
		individual->parameters->reaction_rate_i = MAX(0, MIN(1, individual->parameters->reaction_rate_i + getRandom(-0.05, 0.05)));
	}
	if(force == 1 || getRandom(0, 1) < 0.01)
	{
		individual->parameters->diffusion_speed_a = MAX(0, individual->parameters->diffusion_speed_a + (int) getRandom(-2, 2));
	}
	if(force == 1 || getRandom(0, 1) < 0.01)
	{
		individual->parameters->diffusion_speed_i = MAX(0, individual->parameters->diffusion_speed_i + (int) getRandom(-2, 2));
	}
	if(force == 1 || getRandom(0, 1) < 0.01)
	{
		individual->parameters->reduction_rate = MAX(0, MIN(1, individual->parameters->reduction_rate + getRandom(-0.05, 0.05)));
	}
	if(force == 1 || getRandom(0, 1) < 0.01)
	{
		individual->parameters->diffusion_rate += MAX(0, MIN(1, individual->parameters->diffusion_rate + getRandom(-0.05, 0.05)));
	}
	for(int i = 0; i < individual->parameters->palette_size; i++)
	{
		if(force == 1 || getRandom(0, 1) < 0.01)
		{
			individual->parameters->palette[i] = MAX(0, MIN(255, individual->parameters->palette[i] + (int) getRandom(-2, 2)));
		}
	}
}

Individual * makeBaby(Individual * parent1, Individual * parent2)
{
	int cutIndex = (int) getRandom(0, PARAMETERS_COUNT + 1);
	Individual * child = IndividualCopy(parent1);
	if(cutIndex <= 0)
	{
		child->parameters->reaction_rate_a = parent2->parameters->reaction_rate_a;
	}
	if(cutIndex <= 1)
	{
		child->parameters->reaction_rate_i = parent2->parameters->reaction_rate_i;
	}
	if(cutIndex <= 2)
	{
		child->parameters->diffusion_speed_a = parent2->parameters->diffusion_speed_a;
	}
	if(cutIndex <= 3)
	{
		child->parameters->diffusion_speed_i = parent2->parameters->diffusion_speed_i;
	}
	if(cutIndex <= 4)
	{
		child->parameters->reduction_rate = parent2->parameters->reduction_rate;
	}
	if(cutIndex <= 5)
	{
		child->parameters->diffusion_rate = parent2->parameters->diffusion_rate;
	}
	if(cutIndex <= 6)
	{
		memcpy(child->parameters->palette, parent2->parameters->palette, sizeof(uint8_t) * parent2->parameters->palette_size);
	}
	
	return child;
}
