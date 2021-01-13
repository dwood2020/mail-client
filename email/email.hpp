/*================================================================
* Top level header file
* To be included in external projects only
================================================================*/

// Check this:
// https://codereview.stackexchange.com/questions/139784/sending-email-using-libcurl
// for MIME messages use a proper Framework: mimetic or vmime.org

//TODO Debug session to track where and when the memory allocation takes place (~2 MB)


#pragma once

#include "Address.h"
#include "Client.h"
#include "DateTime.h"
#include "Message.h"

// required std-headers:
#include <string>
#include <vector>

