/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_COLOR_CONVERSION_SYSTEM_H_
#define _KO_COLOR_CONVERSION_SYSTEM_H_

class KoColorProfile;
class KoColorSpace;
class KoColorSpaceFactory;
class KoColorSpaceEngine;
class KoID;

#include "KoColorConversionTransformation.h"

#include <QList>
#include <QPair>

#include "pigment_export.h"

/**
 * This class hold the logic related to pigment's Color Conversion System. It's
 * basically a graph containing all the possible color transformation between
 * the color spaces. The most useful functions are createColorConverter to create
 * a color conversion between two color spaces, and insertColorSpace which is called
 * by KoColorSpaceRegistry each time a new color space is added to the registry.
 *
 * This class is not part of public API, and can be changed without notice.
 */
class PIGMENTCMS_EXPORT KoColorConversionSystem {
    public:
        struct Node;
        struct Vertex;
        struct NodeKey;
        friend uint qHash(const KoColorConversionSystem::NodeKey &key);
        struct Path;
        /**
         * Construct a Color Conversion System, leave to the KoColorSpaceRegistry to
         * create it.
         */
        KoColorConversionSystem();
        ~KoColorConversionSystem();
        /**
         * This function is called by the KoColorSpaceRegistry to add a new color space
         * to the graph of transformation.
         */
        void insertColorSpace(const KoColorSpaceFactory*);
        
        void insertColorProfile( const KoColorProfile* );
        /**
         * This function is called by the color space to create a color conversion
         * between two color space. This function search in the graph of transformations
         * the best possible path between the two color space.
         */
        KoColorConversionTransformation* createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
        
        /**
         * This function creates two transformations, one from the color space and one to the
         * color space. The destination color space is picked from a list of color space, such
         * as the conversion between the two color space is of the best quality.
         * 
         * The typical use case of this function is for KoColorTransformationFactory which
         * doesn't support all color spaces, so unsupported color space have to find an
         * acceptable conversion in order to use that KoColorTransformationFactory.
         * 
         * @param colorSpace the source color space
         * @param possibilities a list of color space among which we need to find the best
         *                      conversion
         * @param fromCS the conversion from the source color space will be affected to this
         *               variable
         * @param toCS the revert conversion to the source color space will be affected to this
         *             variable
         */
        void createColorConverters(const KoColorSpace* colorSpace, QList< QPair<KoID, KoID> >& possibilities, KoColorConversionTransformation*& fromCS, KoColorConversionTransformation*& toCS) const;
    public:
        /**
         * This function return a text that can be compiled using dot to display
         * the graph of color conversion connection.
         */
        QString toDot() const;
        /**
         * This function return a text that can be compiled using dot to display
         * the graph of color conversion connection, with a red link to show the
         * path of the best color conversion.
         */
        QString bestPathToDot(const QString& srcKey, const QString& dstKey) const;
    public:
        /**
         * @return true if there is a path between two color spaces
         */
        bool existsPath( const QString& srcModelId, const QString& srcDepthId, const QString& srcProfileName, const QString& dstModelId, const QString& dstDepthId, const QString& dstProfileName ) const;
        /**
         * @return true if there is a good path between two color spaces
         */
        bool existsGoodPath( const QString& srcModelId, const QString& srcDepthId, const QString& srcProfileName, const QString& dstModelId, const QString& dstDepthId, const QString& dstProfileName ) const;
    private:
        QString vertexToDot(Vertex* v, QString options) const;
    private:
        /**
         * Insert an engine.
         */
        Node* insertEngine(const KoColorSpaceEngine* engine );
        KoColorConversionTransformation* createTransformationFromPath(const Path* path, const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
        /**
         * Query the registry to get the color space associated with this
         * node. (default profile)
         */
        const KoColorSpace* defaultColorSpaceForNode(const Node* node) const;
        /**
         * Create a new node
         */
        Node* createNode( const QString& _modelId, const QString& _depthId, const QString& _profileName );
        /**
         * Initialise a node for ICC color spaces
         */
        void connectToEngine( Node* _node, Node* _engine  );
        const Node* nodeFor( const KoColorSpace*) const;
        /**
         * @return the node corresponding to that key, or create it if needed
         */
        Node* nodeFor(const NodeKey& key);
        const Node* nodeFor(const NodeKey& key) const;
        /**
         * @return the list of nodes that correspond to a given model and depth.
         */
        QList<Node*> nodesFor( const QString& _modelId, const QString& _depthId );
        /**
         * @return the node associated with that key, and create it if needed
         */
        Node* nodeFor(const QString& colorModelId, const QString& colorDepthId, const QString& _profileName);
        const Node* nodeFor(const QString& colorModelId, const QString& colorDepthId, const QString& _profileName) const;
        /**
         * @return the vertex between two nodes, or null if the vertex doesn't exist
         */
        Vertex* vertexBetween(Node* srcNode, Node* dstNode);
        /**
         * create a vertex between two nodes and return it.
         */
        Vertex* createVertex(Node* srcNode, Node* dstNode);
        /**
         * looks for the best path between two nodes
         */
        Path* findBestPath(const Node* srcNode, const Node* dstNode) const;
        /**
         * Delete all the pathes of the list given in argument.
         */
        void deletePathes(QList<KoColorConversionSystem::Path*> pathes) const;
        /**
         * Don't call that function, but raher findBestPath
         * @internal
         */
        inline Path* findBestPathImpl2(const Node* srcNode, const Node* dstNode, bool ignoreHdr, bool ignoreColorCorrectness) const;
        /**
         * Don't call that function, but raher findBestPath
         * @internal
         */
        inline Path* findBestPathImpl(const Node* srcNode, const Node* dstNode, bool ignoreHdr) const;
        
    private:
        struct Private;
        Private* const d;
};

#endif
