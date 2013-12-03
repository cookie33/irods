/* -*- mode: c++; fill-column: 132; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsFileRename.c - server routine that handles the fileRename
 * API
 */

/* script generated code */
#include "fileRename.hpp"
#include "miscServerFunct.hpp"
#include "fileOpr.hpp"
#include "dataObjOpr.hpp"
#include "physPath.hpp"

// =-=-=-=-=-=-=-
#include "irods_log.hpp"
#include "irods_file_object.hpp" 
#include "irods_stacktrace.hpp"
#include "irods_resource_backport.hpp"

int
rsFileRename (rsComm_t *rsComm, fileRenameInp_t *fileRenameInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    //remoteFlag = resolveHost (&fileRenameInp->addr, &rodsServerHost);
    irods::error ret = irods::get_host_for_hier_string( fileRenameInp->rescHier, remoteFlag, rodsServerHost );
    if( !ret.ok() ) {
        irods::log( PASSMSG( "failed in call to irods::get_host_for_hier_string", ret ) );
        return -1;
    }

    if (remoteFlag == LOCAL_HOST) {
        status = _rsFileRename (rsComm, fileRenameInp, rodsServerHost);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteFileRename (rsComm, fileRenameInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog( LOG_NOTICE, "rsFileRename: resolveHost returned unrecognized value %d",
                     remoteFlag );
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteFileRename (rsComm_t *rsComm, fileRenameInp_t *fileRenameInp, 
                  rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
                 "remoteFileRename: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcFileRename (rodsServerHost->conn, fileRenameInp);

    if (status < 0) { 
        rodsLog (LOG_NOTICE,
                 "remoteFileRename: rcFileRename failed for %s",
                 fileRenameInp->newFileName);
    }

    return status;
}

// =-=-=-=-=-=-=-
// local function which makes the call to rename via the resource plugin
int _rsFileRename(
    rsComm_t*         _comm, 
    fileRenameInp_t*  _rename_inp, 
    rodsServerHost_t* _server_host ) {
    // =-=-=-=-=-=-=-
    // FIXME: need to check resource permission and vault permission
    // when RCAT is available 
  
    // mkDirForFilePath( _comm, "/", _rename_inp->newFileName, getDefDirMode () ); - The actual file path depends on the resource

    if(_rename_inp->objPath[0] == '\0') {
        std::stringstream msg;
        msg << __FUNCTION__;
        msg << " - Empty logical path.";
        irods::log(LOG_ERROR, msg.str());
        return -1;
    }
    
    // =-=-=-=-=-=-=-
    // make the call to rename via the resource plugin
    irods::file_object_ptr file_obj( 
                                new irods::file_object( 
                                    _comm, 
                                    _rename_inp->objPath, 
                                    _rename_inp->oldFileName, 
                                    _rename_inp->rescHier, 
                                    0, 0, 0 ) );
    irods::error rename_err = fileRename( _comm, file_obj, _rename_inp->newFileName );

    // =-=-=-=-=-=-=-
    // report errors if any
    if( !rename_err.ok() ) {
        std::stringstream msg;
        msg << "fileRename failed for [";
        msg << _rename_inp->oldFileName;
        msg << "] to [";
        msg << _rename_inp->newFileName;
        msg << "]";
        irods::error err = PASSMSG( msg.str(), rename_err );
        irods::log ( err );
    }

    return rename_err.code();

} // _rsFileRename
 
