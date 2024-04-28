#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "license.h"

#include "comm.h"
#include "filedef.h"
#include "globals.h"
#include "filesub.h"
#include "sdiosubs.h"
#include "ymodem.h"

//	MountSD - Initialize and mount SD card.
//	---------------------------------------
//
//	Unmounts any previous filesystem.
//

void MountSD( char *args[])
{

  (void) args;

  f_mount( 0, "", 0);			// unmount any previous card
  if (SD_Init() == SD_ERR_SUCCESS)	// initialize sd card
  {
    if ( f_mount( &SDfs, "", 0) != FR_OK)
      Uprintf( "\nERROR cannot find filesystem!\n");
    else
      strcpy( CurrentPath, "/");      
  }
  else
    Uprintf("\nSD card not present\n");
  return;
} // MountSD


//	ChangeDir - Set current path.
//	-----------------------------
//
//	Path must exist.
//

void ChangeDir( char *args[])
{

  if ( *args[0])
  {  // if we have a name
    
    if ( f_chdir( args[0]) == FR_OK)
    {    
      f_getcwd( CurrentPath, sizeof(CurrentPath)); 
    }
    else
      Uprintf( "Error - path not found.\n");
  }
  Uprintf( "Current directory is %s\n", CurrentPath);
  return; 
} // ChangeDir

//	MakeDir - Create a directory.
//	-----------------------------
//
//	Usual thing about where it's created.
//

void MakeDir( char *args[])
{

  if ( *args[0])
  {  // if we have a name
    if ( f_mkdir( args[0]) != FR_OK)
      Uprintf( "Error - could not create directory.\n");
  }
  else
    Uprintf("\nA directory name is required.\n");

  return; 
} // MakeDir


//	ShowFiles - Show file directory on SDCard.
//	------------------------------------------
//
//	An optional filespec is allowed.
//
 
void ShowFiles( char *args[])
{

  FRESULT
    fres;
  FATFS
    *fsret;		// used by f_getfree
  FILINFO
    finfo;
  DIR
    dirObj;
  uint32_t
    free_clusters;
  char
    *ch;   
   
  if (args[0] == 0)
    ch = "*";		// null string    
  else
    ch = args[0];	// whatever was requested.    
    
  fres = f_findfirst( &dirObj, &finfo, CurrentPath, ch);  // start searching
  if ( fres != FR_OK)
  { // find first didn't work
    Uprintf( "Error  %d\n", fres);
    return;
  }
  else 
  {
    if ( strlen( finfo.fname) == 0)
    {
      Uprintf( "File not found.\n");
      return;
    }
  }
  
// Loop through the directory, showing files.

  Uprintf( "\nCurrent Directory: %s:\n\n", CurrentPath); 
    
  while ( (fres == FR_OK) && finfo.fname[0])
  {
    Uprintf( " %02d/%02d/%04d  %02d:%02d:%02d  ",
        ((finfo.fdate  >> 5) & 15),
        (finfo.fdate & 31),
        ((finfo.fdate >> 9) + 1980),
        ((finfo.ftime >> 11) & 31),
        ((finfo.ftime >> 5) & 63),
        ((finfo.ftime << 1) & 63));
    Uprintf( "%8d  ", (uint32_t) finfo.fsize);
    Uprintf( "%c", finfo.fattrib & AM_DIR ? '/' : ' ');  
    Uprintf( "%s\n", finfo.fname);
    fres = f_findnext( &dirObj, &finfo);
  } // while

  f_closedir( &dirObj);

// Get and show the free space.

  fres = f_getfree( "", &free_clusters, &fsret);
  if ( fres != FR_OK)
    Uprintf( "\nFailed to get free space size = %d\n", fres);
  else
  {
    unsigned long free = (free_clusters * fsret->csize) / 2;
    
    if ( free > 1024)
      Uprintf( "\n%d MB free\n", free/1000);
    else  
      Uprintf( "\n%d KB free\n", free);
  }
  return;
} // ShowFiles

//	DeleteFile - Delete the named file.
//	-----------------------------------
//
//	We require exactly one argument.
//	


void DeleteFile( char *args[])
{

 FRESULT
    fres;               // file result codes
 FILINFO 
    fno;                // file information return
  DIR
    fdir;               // directory information structure

//	We need at least one argument.

  if (!args[0])
  {
    Uprintf( "File name missing\n");
    return;
  }  // just exit

//  Mount the sd card.

  fres = f_findfirst( &fdir, &fno, CurrentPath, args[0]);
  if ( fres == FR_OK && fno.fname[0] )
  { // if got a hit
  
//	Loop through the directory for matches and delete them.

    do
    {
      Uprintf( "Found: %s\n", fno.fname);
      f_unlink(fno.fname);
      Uprintf( "%s deleted.\n", fno.fname);
      fres = f_findnext( &fdir, &fno);  // go get next one    
    } while ( fres == FR_OK && fno.fname[0]);  // while there are files
  } // if found the first
  else
    Uprintf( "Delete failed.\n");

  return;
} // DeleteFile

//*     MoveFile - Move or rename a file.
//      --------------------------------
//
//      Requires 2 arguments.
//

void MoveFile( char *args[])
{

 FRESULT
    fres;               // file result codes
 FILINFO 
    finfo;                // file information return
  DIR
    fdir;               // directory information structure
  bool
    dirTarget;          // if the target is directory and exists  
  char
    targetName[32];     // we should do something about this

  if (!args[0] || !args[1])
  {
    Uprintf( "Need two arguments--from and to\n");
    return;
  }  // just exit

//      The first argument must exist.

  fres = f_findfirst( &fdir, &finfo, CurrentPath, args[0]);  // start searching
  if ( fres != FR_OK)
  { // find first didn't work
    Uprintf( "Error  %d\n", fres);
    return;
  }
  else 
  {
    if ( strlen( finfo.fname) == 0)
    {
      Uprintf( "File %s not found.\n", args[0]);
      return;
    }
 } // check for first file
  
//  if the second argument exists and is a directory, we make note of it when
//  creating target names.  Otherwise, the target item must not exist.  Rather 
//  than doing an f_stat, it's easiest to do another findfirst.
  
  dirTarget = false;
  fres = f_findfirst( &fdir, &finfo, CurrentPath, args[1]);  // start searching
  if ( fres == FR_OK)
  {
    if ( strlen( finfo.fname) != 0)
    {
      if ( finfo.fattrib & AM_DIR )
        dirTarget = true;               // moving to a directory
      else
      {
        Uprintf( "Error - target %s already exists\n", args[1]);
        return;
      }
    } // if something was returned
  } // if find first okay
  
// okay, so now we can do a find on the first argment.  If we're not moving to
// a directory, we quit with the first rename.

  fres = f_findfirst( &fdir, &finfo, CurrentPath, args[0]);  // start searching
  if ( !dirTarget)
  { // simple rename
    fres = f_rename( args[0], args[1]);
    if (fres != FR_OK)
      Uprintf( "Move/Rename failed.\n");
    return;
  } // check for first file

//  It's a move to a directory, so we form a newname

  do
  {
    strcpy( targetName, args[1]);
    strcat( targetName, "/");
    strcat( targetName, finfo.fname);  
    fres = f_rename( finfo.fname, targetName);
    if ( fres != FR_OK)
      Uprintf( "Error move of %s to %s failed.\n", finfo.fname, targetName);
    else
      Uprintf( "Moved %s to %s\n", finfo.fname, targetName);
    fres = f_findnext( &fdir, &finfo);
  } while ( fres == FR_OK && finfo.fname[0]);  // while there are files
  return;
} // MoveFile

//*	GetFile - Get File via YMODEM.
//	------------------------------
//

void GetFile( char *args[])
{

  int yresult;

  (void) args;

  Uprintf( "\nReady to receive ymodem...");
    
  switch ( (yresult = ReceiveYmodem()) )
  {
  
    case XERR_MOUNT_ERROR:
      Uprintf( "\nCould not mount SD card files.\n");
      break;
    
    case XERR_NO_FILE:
      Uprintf( "\nFile %s could not be created.\n", args[0]);
      break;
    
    case XERR_TIMEOUT:
      Uprintf( "\nTime-out exceeded waiting for sender.\n");
      break;
    
    case XERR_ABORT:
      Uprintf( "\nTransfer cancelled.\n");
      break;

    case XERR_SUCCESS:
       Uprintf( "\nFile transferred.\n");
       break;

    default:
      Uprintf( "\nUnknown error!\n");
      break;
  
  } // case on result of send
  
  return;
} // GetFile

//*	SendFile - Send File via YMODEM.
//	--------------------------------
//
//	Binary only.
//

void SendFile( char *args[])
{

  int yresult;

  if ( !args[0])
  { // no argument
    Uprintf( "Error - the file name must be specified.\n");
    return;
  }
  
  Uprintf( "\nReady to send ymodem %s...", args[0]);
    
  switch ( (yresult = SendYmodem( args[0]))  )
  {
  
    case XERR_MOUNT_ERROR:
      Uprintf( "\nCould not mount SD card files.\n");
      break;
    
    case XERR_NO_FILE:
      Uprintf( "\nFile %s not found.\n", args[0]);
      break;
    
    case XERR_TIMEOUT:
      Uprintf( "\nTime-out exceeded waiting for receiver.\n");
      break;
    
    case XERR_ABORT:
      Uprintf( "\nTransfer cancelled.\n");
      break;

    case XERR_SUCCESS:
       Uprintf( "\nFile transferred.\n");
       break;

    default:
      Uprintf( "\nUnknown error!\n");
      break;
  
  } // case on result of send
  
  return;
} // SendFile
