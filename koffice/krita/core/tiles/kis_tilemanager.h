/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_TILEMANAGER_H_
#define KIS_TILEMANAGER_H_

#include <sys/types.h>

#include <qglobal.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qmutex.h>

#include <ktempfile.h>

class KisTile;
class KisTiledDataManager;

/**
 * This class keeps has the intention to make certain tile-related operations faster or more
 * efficient. It does this by keeping lots of info on KisTiles, and manages the way they are
 * created, used, etc.
 * It mainly does the following more visible things
 *  * provide a way to store tiles on disk to a swap file, to reduce memory usage
 *  * keep a list of previously swapped (but now unused) tiles, to reuse these when we want
 *    to swap new tiles.
 *  * tries to preallocate and recycle some tiles to make future allocations faster
 *    (not done yet)
 */
class KisTileManager  {
public:
    ~KisTileManager();
    static KisTileManager* instance();

public: // Tile management
    void registerTile(KisTile* tile);
    void deregisterTile(KisTile* tile);
    // these can change the tile indirectly, though, through the actual swapping!
    void ensureTileLoaded(const KisTile* tile);
    void maySwapTile(const KisTile* tile);

public: // Pool management
    Q_UINT8* requestTileData(Q_INT32 pixelSize);
    void dontNeedTileData(Q_UINT8* data, Q_INT32 pixelSize);

public: // Configuration
    void configChanged();

private:
    KisTileManager();
    KisTileManager(KisTileManager&) {}
    KisTileManager operator=(const KisTileManager&);

private:
    static KisTileManager *m_singleton;

    // For use when any swap-allocating function failed; the risk of swap allocating failing
    // again is too big, and we'd clutter the logs with kdWarnings otherwise
    bool m_swapForbidden;

    // This keeps track of open swap files, and their associated filesizes
    struct TempFile {
        KTempFile* tempFile;
        off_t fileSize;
    };
    // validNode says if you can swap it (true) or not (false) mmapped, if this tile
    // currently is memory mapped. If it is false, but onFile, it is on disk,
    // but not mmapped, and should be mapped!
    // filePos is the position inside the file; size is the actual size, fsize is the size
    // being used in the swap for this tile (may be larger!)
    // The file points to 0 if it is not swapped, and to the relevant TempFile otherwise
    struct TileInfo { KisTile *tile; KTempFile* file; off_t filePos; int size; int fsize;
        QValueList<TileInfo*>::iterator node;
        bool inMem; bool onFile; bool mmapped; bool validNode; };
    typedef struct { KTempFile* file; off_t filePos; int size; } FreeInfo;
    typedef QMap<const KisTile*, TileInfo*> TileMap;
    typedef QValueList<TileInfo*> TileList;
    typedef QValueList<FreeInfo*> FreeList;
    typedef QValueVector<FreeList> FreeListList;
    typedef QValueList<Q_UINT8*> PoolFreeList;
    typedef QValueList<TempFile> FileList;


    TileMap m_tileMap;
    TileList m_swappableList;
    FreeListList m_freeLists;
    FileList m_files;
    Q_INT32 m_maxInMem;
    Q_INT32 m_currentInMem;
    Q_UINT32 m_swappiness;
    Q_INT32 m_tileSize; // size of a tile if it used 1 byte per pixel
    unsigned long m_bytesInMem;
    unsigned long m_bytesTotal;

    Q_UINT8 **m_pools;
    Q_INT32 *m_poolPixelSizes;
    Q_INT32 m_tilesPerPool;
    PoolFreeList *m_poolFreeList;
    QMutex * m_poolMutex;
    QMutex * m_swapMutex;

    // This is the constant that we will use to see if we want to add a new tempfile
    // We use 1<<30 (one gigabyte) because apparently 32bit systems don't really like very
    // large files.
    static const long MaxSwapFileSize = 1<<30; // For debugging purposes: 1<<20 is a megabyte

    // debug
    int counter;

private:
    void fromSwap(TileInfo* info);
    void toSwap(TileInfo* info);
    void doSwapping();
    void printInfo();
    Q_UINT8* findTileFor(Q_INT32 pixelSize);
    bool isPoolTile(Q_UINT8* data, Q_INT32 pixelSize);
    void reclaimTileToPool(Q_UINT8* data, Q_INT32 pixelSize);

    // Mmap wrapper that prints warnings on error. The result is stored in the *& result
    // the return value is true on succes, false on failure. Other args as in man mmap
    bool kritaMmap(Q_UINT8*& result, void *start, size_t length,
                   int prot, int flags, int fd, off_t offset);
};

#endif // KIS_TILEMANAGER_H_
