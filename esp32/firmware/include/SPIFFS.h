/*
 * Sur la montre, l'app range ses fichiers (cache des cours, progression, reglages) dans la SPIFFS.
 * Sur la T-HMI c'est la carte SD : SPIFFS est ici un autre nom de SD_MMC (meme API : open, exists,
 * totalBytes, usedBytes). Ce fichier masque le SPIFFS.h du framework, voulu.
 */
#ifndef _SPIFFS_SHIM_H
    #define _SPIFFS_SHIM_H

    #include <FS.h>
    #include <SD_MMC.h>

    #define SPIFFS SD_MMC

#endif // _SPIFFS_SHIM_H
