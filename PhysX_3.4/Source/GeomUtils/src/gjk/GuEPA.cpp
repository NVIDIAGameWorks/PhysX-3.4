// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.


#include "GuEPA.h"
#include "GuEPAFacet.h"
#include "GuGJKSimplex.h"
#include "CmPriorityQueue.h"
#include "PsAllocator.h"

#define EPA_DEBUG	0

namespace physx
{
namespace Gu
{
	using namespace Ps::aos;

	class ConvexV;

	struct FacetDistanceComparator
	{
		bool operator()(const Facet* left, const Facet* right) const
		{
			return *left < *right;
		}
	};


#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif


	class EPA
	{	
	public:
		EPA(){}	
		GjkStatus PenetrationDepth(const GjkConvex& a, const GjkConvex& b, const Ps::aos::Vec3V* PX_RESTRICT Q, const Ps::aos::Vec3V* PX_RESTRICT A, const Ps::aos::Vec3V* PX_RESTRICT B, const PxI32 size, Ps::aos::Vec3V& pa, Ps::aos::Vec3V& pb, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penDepth, const bool takeCoreShape = false);
		bool expandPoint(const GjkConvex& a, const GjkConvex& b, PxI32& numVerts, const FloatVArg upperBound);
		bool expandSegment(const GjkConvex& a, const GjkConvex& b, PxI32& numVerts, const FloatVArg upperBound);
		bool expandTriangle(PxI32& numVerts, const FloatVArg upperBound);

		Facet* addFacet(const PxU32 i0, const PxU32 i1, const PxU32 i2, const Ps::aos::FloatVArg upper);
	
		bool originInTetrahedron(const Ps::aos::Vec3VArg p1, const Ps::aos::Vec3VArg p2, const Ps::aos::Vec3VArg p3, const Ps::aos::Vec3VArg p4);

		Cm::InlinePriorityQueue<Facet*, MaxFacets, FacetDistanceComparator> heap;
		Ps::aos::Vec3V aBuf[MaxSupportPoints];
		Ps::aos::Vec3V bBuf[MaxSupportPoints];
		Facet facetBuf[MaxFacets];
		EdgeBuffer edgeBuffer;
		EPAFacetManager facetManager;

	private:
		PX_NOCOPY(EPA)
	};
#if PX_VC 
     #pragma warning(pop) 
#endif

	PX_FORCE_INLINE bool EPA::originInTetrahedron(const Ps::aos::Vec3VArg p1, const Ps::aos::Vec3VArg p2, const Ps::aos::Vec3VArg p3, const Ps::aos::Vec3VArg p4)
	{
		using namespace Ps::aos;
		return BAllEqFFFF(PointOutsideOfPlane4(p1, p2, p3, p4)) == 1;
	}


	static PX_FORCE_INLINE void doSupport(const GjkConvex& a, const GjkConvex& b, const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support)
	{
		const Vec3V tSupportA = a.support(V3Neg(dir));
		const Vec3V tSupportB = b.support(dir);
		//avoid LHS
		supportA = tSupportA;
		supportB = tSupportB;
		support = V3Sub(tSupportA, tSupportB);
	}

	GjkStatus epaPenetration(const GjkConvex& a, const GjkConvex& b, PxU8* PX_RESTRICT aInd, PxU8* PX_RESTRICT bInd, PxU8 _size, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth, const bool takeCoreShape)
	{
		using namespace Ps::aos;
		const BoolV bTrue = BTTTT();
		const Vec3V zeroV = V3Zero();

		PxU32 size=_size;
		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());

		//ML: eps2 is the square value of an epsilon value which applied in the termination condition for two shapes overlap. EPA will terminate based on sq(v) < eps2 and indicate that two shapes are overlap. 
		//we calculate the eps2 based on 10% of the minimum margin of two shapes
		const FloatV eps = FMul(minMargin, FLoad(0.1f));
		const FloatV eps2 = FMul(eps, eps);

		Vec3V A[4];
		Vec3V B[4];
		Vec3V Q[4];
		Vec3V v(zeroV);
		BoolV bNotTerminated  = bTrue;
		BoolV bCon = bTrue;
		FloatV sDist = FMax();
		FloatV minDist= sDist;
		Vec3V supportA, supportB, support;
		FloatV marginDif = FZero();
  
		if(_size != 0)
		{
			//ML: we construct a simplex based on the gjk simplex indices
			for(PxU32 i=0; i<_size; ++i)
			{
				supportA = a.supportPoint(aInd[i], &marginDif);
				supportB = b.supportPoint(bInd[i], &marginDif);
				//avoid LHS in xbox
				A[i] = supportA;
				B[i] = supportB;
				Q[i] = V3Sub(supportA, supportB);
			}

			//ML: if any of the gjk shapes is a shrunk shape, we need to recreate the simplex for the original shape to work on; otherwise, we can just skip this process
			//and go to the epa code
			if(!takeCoreShape)
			{
				v = GJKCPairDoSimplex(Q, A, B, Q[size-1],  size);
				sDist = V3Dot(v, v);	
				bNotTerminated = FIsGrtr(sDist, eps2);
			}
			else
			{
				bNotTerminated = BFFFF();
			}
		}
		else
		{
			const Vec3V _initialSearchDir = V3Sub(a.getCenter(), b.getCenter());
			v = V3Sel(FIsGrtr(V3Dot(_initialSearchDir, _initialSearchDir), FZero()), _initialSearchDir, V3UnitX());
		}

	
		while(BAllEqTTTT(bNotTerminated))
		{
			minDist = sDist;
			//calculate the support point
			//pair->doSupport(v, A[size], B[size], Q[size]);
			supportA = a.support(V3Neg(v));
			supportB = b.support(v);
			support = V3Sub(supportA, supportB);
			A[size] = supportA;
			B[size] = supportB;
			Q[size++] = support;
	
			v = GJKCPairDoSimplex(Q, A, B, support, size);
			sDist = V3Dot(v, v);
			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
		}

		EPA epa;

		return epa.PenetrationDepth(a, b, Q, A, B, PxI32(size), contactA, contactB, normal, penetrationDepth, takeCoreShape);
	}

	//ML: this function returns the signed distance of a point to a plane
	PX_FORCE_INLINE Ps::aos::FloatV Facet::getPlaneDist(const Ps::aos::Vec3VArg p, const Ps::aos::Vec3V* PX_RESTRICT aBuf, const Ps::aos::Vec3V* PX_RESTRICT bBuf) const
	{
		const Vec3V pa0(aBuf[m_indices[0]]);
		const Vec3V pb0(bBuf[m_indices[0]]);
		const Vec3V p0 = V3Sub(pa0, pb0);

		return V3Dot(m_planeNormal, V3Sub(p, p0));
	}

	//ML: This function:
	// (1)calculates the distance from orign((0, 0, 0)) to a triangle plane 
	// (2) rejects triangle if the triangle is degenerate (two points are identical)
	// (3) rejects triangle to be added into the heap if the plane distance is large than upper
	Ps::aos::BoolV Facet::isValid2(const PxU32 i0, const PxU32 i1, const PxU32 i2, const Ps::aos::Vec3V* PX_RESTRICT aBuf, const Ps::aos::Vec3V* PX_RESTRICT bBuf, 
		const Ps::aos::FloatVArg upper)
	{
		using namespace Ps::aos;
		const FloatV eps = FEps();

		const Vec3V pa0(aBuf[i0]);
		const Vec3V pa1(aBuf[i1]);
		const Vec3V pa2(aBuf[i2]);

		const Vec3V pb0(bBuf[i0]);
		const Vec3V pb1(bBuf[i1]);
		const Vec3V pb2(bBuf[i2]);

		const Vec3V p0 = V3Sub(pa0, pb0);
		const Vec3V p1 = V3Sub(pa1, pb1);
		const Vec3V p2 = V3Sub(pa2, pb2);

		const Vec3V v1 = V3Sub(p1, p0);
		const Vec3V v2 = V3Sub(p2, p0);

		const Vec3V denormalizedNormal = V3Cross(v1, v2);
		FloatV norValue = V3Dot(denormalizedNormal, denormalizedNormal);
		//if norValue < eps, this triangle is degenerate
		const BoolV con = FIsGrtr(norValue, eps);
		norValue = FSel(con, norValue, FOne());
	
		const Vec3V planeNormal = V3Scale(denormalizedNormal, FRsqrt(norValue));
		const FloatV planeDist =  V3Dot(planeNormal, p0);
		m_planeNormal = planeNormal;
		
		FStore(planeDist, &m_planeDist);

		return BAnd(con, FIsGrtrOrEq(upper, planeDist));
		//return BAnd(con, BAnd(FIsGrtrOrEq(planeDist, lower), FIsGrtrOrEq(upper, planeDist)));
	}

	//ML: if the triangle is valid(not degenerate and within lower and upper bound), we need to add it into the heap. Otherwise, we just return
	//the triangle so that the facet can be linked to other facets in the expanded polytope.
	Facet* EPA::addFacet(const PxU32 i0, const PxU32 i1, const PxU32 i2, const Ps::aos::FloatVArg upper)
	{
		using namespace Ps::aos;
		PX_ASSERT(i0 != i1 && i0 != i2 && i1 != i2);
		//ML: we move the control in the calling code so we don't need to check weather we will run out of facets or not
		PX_ASSERT(facetManager.getNumUsedID() < MaxFacets);

		const PxU32 facetId = facetManager.getNewID();
		Ps::prefetchLine(&facetBuf[facetId], 128);

		Facet * facet = PX_PLACEMENT_NEW(&facetBuf[facetId],Facet(i0, i1, i2));
		facet->m_FacetId = PxU8(facetId);

		const  BoolV validTriangle = facet->isValid2(i0, i1, i2, aBuf, bBuf, upper);

		if(BAllEqTTTT(validTriangle))
		{
			heap.push(facet);
			facet->m_inHeap = true;
		}
		else
		{
			facet->m_inHeap = false;
		}
		return facet;
	}

	//ML: this function performs a flood fill over the boundary of the current polytope. 
	void Facet::silhouette(const PxU32 _index, const Ps::aos::Vec3VArg w, const Ps::aos::Vec3V* PX_RESTRICT aBuf, const Ps::aos::Vec3V* PX_RESTRICT bBuf, EdgeBuffer& edgeBuffer, EPAFacetManager& manager) 
	{
		using namespace Ps::aos;
		const FloatV zero = FZero();
		Edge stack[MaxFacets];
		stack[0] = Edge(this, _index);
		PxI32 size = 1;
		while(size--)
		{
			Facet* const PX_RESTRICT f = stack[size].m_facet;
			const PxU32 index = stack[size].m_index;
			PX_ASSERT(f->Valid());
			
			if(!f->m_obsolete)
			{
				//ML: if the point is above the facet, the facet has an reflex edge, which will make the polytope concave. Therefore, we need to 
				//remove this facet and make sure the expanded polytope is convex.
				const FloatV pointPlaneDist = f->getPlaneDist(w, aBuf, bBuf);
				
				if(FAllGrtr(zero, pointPlaneDist))
				{
					//ML: facet isn't visible from w (we don't have a reflex edge), this facet will be on the boundary and part of the new polytope so that
					//we will push it into our edgeBuffer
					if(!edgeBuffer.Insert(f, index))
						return;
				} 
				else 
				{
					//ML:facet is visible from w, therefore, we need to remove this facet from the heap and push its adjacent facets onto the stack
					f->m_obsolete = true; // Facet is visible from w
					const PxU32 next(incMod3(index));
					const PxU32 next2(incMod3(next));
					stack[size++] = Edge(f->m_adjFacets[next2],PxU32(f->m_adjEdges[next2]));
					stack[size++] = Edge(f->m_adjFacets[next], PxU32(f->m_adjEdges[next]));
					
					PX_ASSERT(size <= MaxFacets);
					if(!f->m_inHeap)
					{
						//if the facet isn't in the heap, we can release that memory
						manager.deferredFreeID(f->m_FacetId);
					}
				}
			}
		}
	}

	//ML: this function perform flood fill for the adjancent facet and store the boundary facet into the edgeBuffer
	void Facet::silhouette(const Ps::aos::Vec3VArg w, const Ps::aos::Vec3V* PX_RESTRICT aBuf, const Ps::aos::Vec3V* PX_RESTRICT bBuf, EdgeBuffer& edgeBuffer, EPAFacetManager& manager)
	{
		m_obsolete = true;
		for(PxU32 a = 0; a < 3; ++a)
		{
			m_adjFacets[a]->silhouette(PxU32(m_adjEdges[a]), w, aBuf, bBuf, edgeBuffer, manager);
		}
	}

	bool EPA::expandPoint(const GjkConvex& a, const GjkConvex& b, PxI32& numVerts, const FloatVArg upperBound)
	{
		const Vec3V x = V3UnitX();
		Vec3V q0 = V3Sub(aBuf[0], bBuf[0]);
		Vec3V q1;
		doSupport(a, b, x, aBuf[1], bBuf[1], q1);
		if (V3AllEq(q0, q1))
			return false;
		return expandSegment(a, b, numVerts, upperBound);
	}

	//ML: this function use the segement to create a triangle
	bool EPA::expandSegment(const GjkConvex& a, const GjkConvex& b, PxI32& numVerts, const FloatVArg upperBound)
	{
		const Vec3V q0 = V3Sub(aBuf[0], bBuf[0]);
		const Vec3V q1 = V3Sub(aBuf[1], bBuf[1]);
		const Vec3V v = V3Sub(q1, q0);
		const Vec3V absV = V3Abs(v);

		const FloatV x = V3GetX(absV);
		const FloatV y = V3GetY(absV);
		const FloatV z = V3GetZ(absV);

		Vec3V axis = V3UnitX();
		const BoolV con0 = BAnd(FIsGrtr(x, y), FIsGrtr(z, y));
		if (BAllEqTTTT(con0))
		{
			axis = V3UnitY();
		}
		else if(FAllGrtr(x, z))
		{
			axis = V3UnitZ();
		}
		
		const Vec3V n = V3Normalize(V3Cross(axis, v));
		Vec3V q2;
		doSupport(a, b, n, aBuf[2], bBuf[2], q2);

		return expandTriangle(numVerts, upperBound);
	}

	bool EPA::expandTriangle(PxI32& numVerts, const FloatVArg upperBound)
	{
		numVerts = 3;

		Facet * PX_RESTRICT f0 = addFacet(0, 1, 2, upperBound);
		Facet * PX_RESTRICT f1 = addFacet(1, 0, 2, upperBound);

		if(heap.empty())
			return false;

		f0->link(0, f1, 0);
		f0->link(1, f1, 2);
		f0->link(2, f1, 1);

		return true;
	}


	//ML: this function calculates contact information. If takeCoreShape flag is true, this means the two closest points will be on the core shape used in the support functions. 
	//For example, we treat sphere/capsule as a point/segment in the support function for GJK/EPA, so that the core shape for sphere/capsule is a point/segment. For PCM, we need 
	//to take the point from the core shape because this will allows us recycle the contacts more stably. For SQ sweeps, we need to take the point on the surface of the sphere/capsule 
	//when we calculate MTD because this is what will be reported to the user. Therefore, the takeCoreShape flag will be set to be false in SQ.
	static void calculateContactInformation(const Ps::aos::Vec3V* PX_RESTRICT aBuf, const Ps::aos::Vec3V* PX_RESTRICT bBuf, Facet* facet, const GjkConvex& a, const GjkConvex& b, Vec3V& pa, Vec3V& pb, Vec3V& normal, FloatV& penDepth, const bool takeCoreShape)
	{
		const FloatV zero = FZero();
		Vec3V _pa, _pb;
		facet->getClosestPoint(aBuf, bBuf, _pa, _pb);
		//dist > 0 means two shapes are penetrated. If dist < 0(when origin isn't inside the polytope), two shapes status are unknown
		const FloatV dist = facet->getPlaneDist();
		
		const Vec3V planeNormal = V3Neg(facet->getPlaneNormal());

		if(takeCoreShape)
		{
			pa = _pa;
			pb = _pb;
			normal = planeNormal;
			penDepth = FNeg(dist);
		}
		else
		{
			
			const BoolV aQuadratic = a.isMarginEqRadius();
			const BoolV bQuadratic = b.isMarginEqRadius();

			const FloatV marginA = FSel(aQuadratic, a.getMargin(), zero);
			const FloatV marginB = FSel(bQuadratic, b.getMargin(), zero);
			const FloatV sumMargin = FAdd(marginA, marginB);
			pa = V3NegScaleSub(planeNormal, marginA, _pa);
			pb = V3ScaleAdd(planeNormal, marginB, _pb);
			normal = planeNormal;
			penDepth = FNeg(FAdd(dist, sumMargin));

#if	EPA_DEBUG

			PxVec3 tempA, tempB;

			V3StoreU(pa, tempA);
			V3StoreU(pb, tempB);

			if (!tempA.isFinite())
			{
				Ps::debugBreak();
			}

			if (!tempB.isFinite())
			{
				Ps::debugBreak();
			}
			const Vec3V v = V3Sub(_pb, _pa);
			const FloatV length = V3Length(v);
			const Vec3V cn = V3ScaleInv(v, length); 
			const Vec3V cnn = V3Sel(FIsGrtr(dist, zero), cn, V3Neg(cn));
			//make sure planeNormal and n are pointing to the same direction
			const FloatV dir = V3Dot(cnn, normal);
			PX_ASSERT(FAllGrtr(dir, zero));
			//planeDist might be zero if the origin didn't include in the polytope
			const FloatV dif = V3Sub(length, FAbs(penDepth));
			PX_ASSERT(FAllGrtr(FLoad(1e-5f), FAbs(dif)));
#endif
		}
	}

	//ML: This function returns one of three status codes:
	//(1)EPA_FAIL:	the algorithm failed to create a valid polytope(the origin wasn't inside the polytope) from the input simplex
	//(2)EPA_CONTACT : the algorithm found the MTD amd converged successfully.
	//(3)EPA_DEGENERATE: the algorithm cannot make further progress and the result is unknown.
	GjkStatus EPA::PenetrationDepth(const GjkConvex& a, const GjkConvex& b, const Ps::aos::Vec3V* PX_RESTRICT /*Q*/, const Ps::aos::Vec3V* PX_RESTRICT A, const Ps::aos::Vec3V* PX_RESTRICT B, const PxI32 size, Ps::aos::Vec3V& pa, Ps::aos::Vec3V& pb, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penDepth, const bool takeCoreShape)
	{
	
		using namespace Ps::aos;   

		Ps::prefetchLine(&facetBuf[0]);
		Ps::prefetchLine(&facetBuf[0], 128);

		const FloatV zero = FZero();
	
		const FloatV _max = FMax();
		FloatV upper_bound(_max);
	
		aBuf[0]=A[0]; aBuf[1]=A[1]; aBuf[2]=A[2]; aBuf[3]=A[3];
		bBuf[0]=B[0]; bBuf[1]=B[1]; bBuf[2]=B[2]; bBuf[3]=B[3];

		PxI32 numVertsLocal = 0;

		heap.clear();

		//if the simplex isn't a tetrahedron, we need to construct one before we can expand it
		switch (size) 
		{
		case 1:
			{
				// Touching contact. Yes, we have a collision and the penetration will be zero
				if(!expandPoint(a, b, numVertsLocal, upper_bound))
					return EPA_FAIL;
				break;
			}
		case 2: 
			{
				// We have a line segment inside the Minkowski sum containing the
				// origin. we need to construct two back to back triangles which link to each other
				if(!expandSegment(a, b, numVertsLocal, upper_bound))
					return EPA_FAIL;
				break;
			}
		case 3: 
			{
				// We have a triangle inside the Minkowski sum containing
				// the origin. We need to construct two back to back triangles which link to each other
				if(!expandTriangle(numVertsLocal, upper_bound))
					return EPA_FAIL;

				break;
				
			}
		case 4:
			{
				//check for input face normal. All face normals in this tetrahedron should be all pointing either inwards or outwards. If all face normals are pointing outward, we are good to go. Otherwise, we need to 
				//shuffle the input vertexes and make sure all face normals are pointing outward
				const Vec3V pa0(aBuf[0]);
				const Vec3V pa1(aBuf[1]);
				const Vec3V pa2(aBuf[2]);
				const Vec3V pa3(aBuf[3]);

				const Vec3V pb0(bBuf[0]);
				const Vec3V pb1(bBuf[1]);
				const Vec3V pb2(bBuf[2]);
				const Vec3V pb3(bBuf[3]);

				const Vec3V p0 = V3Sub(pa0, pb0);
				const Vec3V p1 = V3Sub(pa1, pb1);
				const Vec3V p2 = V3Sub(pa2, pb2);
				const Vec3V p3 = V3Sub(pa3, pb3);

				const Vec3V v1 = V3Sub(p1, p0);
				const Vec3V v2 = V3Sub(p2, p0);

				const Vec3V planeNormal = V3Normalize(V3Cross(v1, v2));

				const FloatV signDist = V3Dot(planeNormal, V3Sub(p3, p0));

				if (FAllGrtr(signDist, zero))
				{
					//shuffle the input vertexes
					const Vec3V tempA0 = aBuf[2];
					const Vec3V tempB0 = bBuf[2];
					aBuf[2] = aBuf[1];
					bBuf[2] = bBuf[1];
					aBuf[1] = tempA0;
					bBuf[1] = tempB0;
				}

				Facet * PX_RESTRICT f0 = addFacet(0, 1, 2, upper_bound);
				Facet * PX_RESTRICT f1 = addFacet(0, 3, 1, upper_bound);
				Facet * PX_RESTRICT f2 = addFacet(0, 2, 3, upper_bound);
				Facet * PX_RESTRICT f3 = addFacet(1, 3, 2, upper_bound);
			
				if (heap.empty())
					return EPA_FAIL;

				PX_ASSERT(f0->m_planeDist >= -1e-2f);
				PX_ASSERT(f1->m_planeDist >= -1e-2f);
				PX_ASSERT(f2->m_planeDist >= -1e-2f);
				PX_ASSERT(f3->m_planeDist >= -1e-2f);

				f0->link(0, f1, 2);
				f0->link(1, f3, 2);
				f0->link(2, f2, 0);
				f1->link(0, f2, 2);
				f1->link(1, f3, 0);
				f2->link(1, f3, 1);
				numVertsLocal = 4;

				break;
			}
		}

	
		const FloatV tenPerc = FLoad(0.1f);
		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());
		const FloatV eps2 = FMul(minMargin, tenPerc);

		Facet* PX_RESTRICT facet = NULL;

		Vec3V tempa, tempb, q;

		do 
		{
	
			facetManager.processDeferredIds();
			facet = heap.pop(); //get the shortest distance triangle of origin from the list
			facet->m_inHeap = false;
			
			if (!facet->isObsolete()) 
			{
				Ps::prefetchLine(edgeBuffer.m_pEdges);
				Ps::prefetchLine(edgeBuffer.m_pEdges,128);
				Ps::prefetchLine(edgeBuffer.m_pEdges,256);

				const Vec3V planeNormal = facet->getPlaneNormal();
				const FloatV planeDist = facet->getPlaneDist();

				tempa = a.support(planeNormal);
				tempb = b.support(V3Neg(planeNormal));

				q = V3Sub(tempa, tempb);
				
				Ps::prefetchLine(&aBuf[numVertsLocal],128);
				Ps::prefetchLine(&bBuf[numVertsLocal],128);

				//calculate the distance from support point to the origin along the plane normal. Because the support point is search along
				//the plane normal, which means the distance should be positive. However, if the origin isn't contained in the polytope, dist
				//might be negative
				const FloatV dist = V3Dot(q, planeNormal);

				const BoolV con0 = FIsGrtrOrEq(eps2, FAbs(FSub(dist, planeDist)));

				if (BAllEqTTTT(con0))
				{
					calculateContactInformation(aBuf, bBuf, facet, a, b, pa, pb, normal, penDepth, takeCoreShape);
					return EPA_CONTACT;
				}

				//update the upper bound to the minimum between existing upper bound and the distance
				upper_bound = FMin(upper_bound, dist);

				aBuf[numVertsLocal]=tempa;
				bBuf[numVertsLocal]=tempb;

				const PxU32 index =PxU32(numVertsLocal++);

				// Compute the silhouette cast by the new vertex
				// Note that the new vertex is on the positive side
				// of the current facet, so the current facet will
				// not be in the polytope. Start local search
				// from this facet.

				edgeBuffer.MakeEmpty();

				facet->silhouette(q, aBuf, bBuf, edgeBuffer, facetManager);

				//the edge buffer either empty or overflow
				if (!edgeBuffer.IsValid())
				{
					calculateContactInformation(aBuf, bBuf, facet, a, b, pa, pb, normal, penDepth, takeCoreShape);
					return EPA_DEGENERATE;
				}

				Edge* PX_RESTRICT edge=edgeBuffer.Get(0);

				PxU32 bufferSize=edgeBuffer.Size();

				//check to see whether we have enough space in the facet manager to create new facets
				if(bufferSize > facetManager.getNumRemainingIDs())
				{
					calculateContactInformation(aBuf, bBuf, facet, a, b, pa, pb, normal, penDepth, takeCoreShape);
					return EPA_DEGENERATE;
				}

				Facet *firstFacet = addFacet(edge->getTarget(), edge->getSource(),index, upper_bound);
				PX_ASSERT(firstFacet);
				firstFacet->link(0, edge->getFacet(), edge->getIndex());

				Facet * PX_RESTRICT lastFacet = firstFacet;

#if EPA_DEBUG
				bool degenerate = false;
				for(PxU32 i=1; (i<bufferSize) && (!degenerate); ++i)
				{
					edge=edgeBuffer.Get(i);
					Facet* PX_RESTRICT newFacet = addFacet(edge->getTarget(), edge->getSource(),index, upper_bound);
					PX_ASSERT(newFacet);
					const bool b0 = newFacet->link(0, edge->getFacet(), edge->getIndex());
					const bool b1 = newFacet->link(2, lastFacet, 1);
					degenerate = degenerate || !b0 || !b1;
					lastFacet = newFacet;
				}

				if (degenerate)
					Ps::debugBreak();
#else
				for (PxU32 i = 1; i<bufferSize; ++i)
				{
					edge = edgeBuffer.Get(i);
					Facet* PX_RESTRICT newFacet = addFacet(edge->getTarget(), edge->getSource(), index, upper_bound);
					newFacet->link(0, edge->getFacet(), edge->getIndex());
					newFacet->link(2, lastFacet, 1);
					lastFacet = newFacet;
				}
#endif

				firstFacet->link(2, lastFacet, 1);
			}
			facetManager.freeID(facet->m_FacetId);

		}
		while((heap.size() > 0) && FAllGrtr(upper_bound, heap.top()->getPlaneDist()) && numVertsLocal != MaxSupportPoints);

		calculateContactInformation(aBuf, bBuf, facet, a, b, pa, pb, normal, penDepth, takeCoreShape);

		return EPA_DEGENERATE;
	}
}

}
