//
// Created by yateu on 10/02/2019.
//

#ifndef TURING_MAIN_H
#define TURING_MAIN_H

#include "individual.h"
#include "generation.h"

int keepParent(int individualIndex, Individual * individual);

void CreateHTML(char * filename, Generation * pGeneration);

void mutate(Individual * individual, int force);

Individual * makeBaby(Individual * parent1, Individual * parent2);

#endif //TURING_MAIN_H
