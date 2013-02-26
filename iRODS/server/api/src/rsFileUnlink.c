/* -*- mode: c++; fill-column: 132; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See fileUnlink.h for a description of this API call.*/

#include "fileUnlink.h"
#include "miscServerFunct.h"

// =-=-=-=-=-=-=-
// eirods include
#include "eirods_log.h"
#include "eirods_file_object.h"
#include "eirods_stacktrace.h"
#include "eirods_resource_backport.h"

int
rsFileUnlink (rsComm_t *rsComm, fileUnlinkInp_t *fileUnlinkInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    //remoteFlag = resolveHost (&fileUnlinkInp->addr, &rodsServerHost);
    eirods::error ret = eirods::get_host_for_hier_string( fileUnlinkInp->rescHier, remoteFlag, rodsServerHost );
    if( !ret.ok() ) {
        eirods::log( PASSMSG( "rsFileUnlink - failed in call to eirods::get_host_for_hier_string", ret ) );
        return -1;
    }
    if (remoteFlag == LOCAL_HOST) {
        status = _rsFileUnlink (rsComm, fileUnlinkInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteFileUnlink (rsComm, fileUnlinkInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
                     "rsFileUnlink: resolveHost returned unrecognized value %d",
                     remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    /* Manually insert call-specific code here */

    return (status);
}

int
remoteFileUnlink (rsComm_t *rsComm, fileUnlinkInp_t *fileUnlinkInp,
                  rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
                 "remoteFileUnlink: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }


    status = rcFileUnlink (rodsServerHost->conn, fileUnlinkInp);

    if (status < 0) { 
        rodsLog (LOG_NOTICE,
                 "remoteFileUnlink: rcFileUnlink failed for %s, status = %d",
                 fileUnlinkInp->fileName, status);
    }

    return status;
}

// =-=-=-=-=-=-=-
// local function for calling unlink via resource plugin
int _rsFileUnlink(
    rsComm_t *rsComm,
    fileUnlinkInp_t *fileUnlinkInp )
{
    if(fileUnlinkInp->objPath[0] == '\0') {
        std::stringstream msg;
        msg << __FUNCTION__;
        msg << " - empty logical path.";
        eirods::log(LOG_ERROR, msg.str());
        return -1;
    }
    
    // =-=-=-=-=-=-=-
    // call unlink via resource plugin
    eirods::file_object file_obj( rsComm, fileUnlinkInp->objPath, fileUnlinkInp->fileName, fileUnlinkInp->rescHier, 0, 0, 0 );
    eirods::error unlink_err = fileUnlink( rsComm, file_obj );
     
    // =-=-=-=-=-=-=-
    // log potential error message
    if( unlink_err.code() < 0 ) {
        std::stringstream msg;
        msg << "_rsFileUnlink: fileRead for ";
        msg << fileUnlinkInp->fileName;
        msg << ", status = ";
        msg << unlink_err.code();
        eirods::error ret_err = PASS( false, unlink_err.code(), msg.str(), unlink_err );
        eirods::log( ret_err );
    }

    return (unlink_err.code());

} // _rsFileUnlink









