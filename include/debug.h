//
// Created by alexander on 5/18/22.
//

#ifndef BVPM_DEBUG_H
#define BVPM_DEBUG_H
#if defined(DEBUG)
#define PRINT_DEBUG(x) (std::cout << x)
#else
#define PRINT_DEBUG(x)
#endif

#endif //BVPM_DEBUG_H
