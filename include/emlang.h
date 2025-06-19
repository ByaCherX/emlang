/*
| Copyright (C) 2025 EMLang Project. All rights reserved.
*/

#ifndef EMLANG_H
#define EMLANG_H

/*------   Version   ------*/
#define EMLANG_VERSION_MAJOR    0
#define EMLANG_VERSION_MINOR    9
#define EMLANG_VERSION_PATCH    26
#define EMLANG_VERSION_NUMBER  (EMLANG_VERSION_MAJOR *100*100 + EMLANG_VERSION_MINOR *100 + EMLANG_VERSION_PATCH)
#define EMLANG_VERSION_STRING   EMLANG_VERSION_MAJOR "." \
                                EMLANG_VERSION_MINOR "." \
                                EMLANG_VERSION_PATCH

// Project Information
#define EMLANG_PROJECT_NAME     "EMLang"
#define EMLANG_COMPANY_NAME     "EMLang Project"
#define EMLANG_COPYRIGHT        "Copyright (C) 2025 EMLang Project. All rights reserved."

// Build Information
#define EMLANG_BUILD_DATE       __DATE__
#define EMLANG_BUILD_TIME       __TIME__

// Comments
#define EMLANG_COMMENTS         "EMLang is a modern programming language with advanced features."

/***************************************
*  Feature flags
***************************************/
#define EMLANG_FEATURE_POINTERS          0
#define EMLANG_FEATURE_CASTING           0
#define EMLANG_FEATURE_IMPORTS           0
#define EMLANG_FEATURE_EXTERN_FUNCTIONS  0


#endif // EMLANG_H
