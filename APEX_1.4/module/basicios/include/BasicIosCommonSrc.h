/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_COMMON_SRC_H__
#define __BASIC_IOS_COMMON_SRC_H__

namespace nvidia
{
	namespace basicios
	{

		PX_CUDA_CALLABLE PX_INLINE void updateCollisionVelocity(const CollisionData& data, const PxVec3& normal, const PxVec3& position, PxVec3& velocity)
		{
			PxVec3 bodyVelocity = data.bodyLinearVelocity + data.bodyAngluarVelocity.cross(position - data.bodyCMassPosition);

			velocity -= bodyVelocity;
			float normalVelocity = normal.dot(velocity);
			if (normalVelocity < 0.0f)
			{
				velocity -= normal * ((1.0f + data.materialRestitution) * normalVelocity);
			}
			velocity += bodyVelocity;
		}


		PX_CUDA_CALLABLE PX_INLINE void calculatePointSegmentSquaredDist(const PxVec3& a, const PxVec3& b, const PxVec3& point, float& distanceSquared, PxVec3& nearestPoint) // a, b - segment points
		{
			PxVec3 v, w, temp; //vectors
			float c1, c2, ratio; //constants
			v = a - b;
			w = point - b;
			float distSquared = 0;

			c1 = w.dot(v);
			if(c1 <= 0) 
			{
				distSquared = (point.x - b.x) *  (point.x - b.x) + (point.y - b.y) * (point.y - b.y) + (point.z - b.z) * (point.z - b.z);
				nearestPoint = b;
			}
			else
			{
				c2 = v.dot(v);
				if(c2 <= c1)
				{
					distSquared = (point.x - a.x) *  (point.x - a.x) + (point.y - a.y) * (point.y - a.y) + (point.z - a.z) * (point.z - a.z);
					nearestPoint = a;
				}
				else
				{
					ratio = c1 / c2;
					temp = b + ratio * v;
					distSquared = (point.x - temp.x) *  (point.x - temp.x) + (point.y - temp.y) * (point.y - temp.y) + (point.z - temp.z) * (point.z - temp.z);
					nearestPoint = temp;
				}
			}

			distanceSquared = distSquared;
		}

#ifdef __CUDACC__
		__device__
#endif
		PX_INLINE float checkTriangleCollision(const PxVec4* memTrimeshVerts, const uint32_t* memTrimeshIndices, const CollisionTriMeshData& data, float radius, PxVec3 localPosition, PxVec3 &normal)
		{
			float minDistSquared = PX_MAX_F32;
			PxVec3 localNormal(0);
					
			for(uint32_t j = 0 ; j < data.numTriangles; j++)
			{
				PxVec3 p0, p1, p2;
				uint32_t i0, i1, i2;
				i0 = SIM_FETCH(TrimeshIndices, data.firstIndex + 3 * j);
				i1 = SIM_FETCH(TrimeshIndices, data.firstIndex + 3 * j + 1);
				i2 = SIM_FETCH(TrimeshIndices, data.firstIndex + 3 * j + 2);
				splitFloat4(p0, SIM_FETCH(TrimeshVerts, data.firstVertex + i0));
				splitFloat4(p1, SIM_FETCH(TrimeshVerts, data.firstVertex + i1));
				splitFloat4(p2, SIM_FETCH(TrimeshVerts, data.firstVertex + i2));

				//localNormal = (p1 - p0).cross(p2 - p0);
				//if(radius > 0) localNormal += localPosition;
						
				PxBounds3 aabb( (p0.minimum(p1.minimum(p2))), (p0.maximum(p1.maximum(p2))) );
				aabb.fattenFast( radius );
				if( !aabb.contains(localPosition) ) continue;

				p0 = p0 - localPosition;
				p1 = p1 - localPosition;
				p2 = p2 - localPosition;

				PxVec3 a(p1 - p0);
				PxVec3 b(p2 - p0);
				PxVec3 n = a.cross(b);	
				n.normalize();

				//check if point far away from the triangle's plane, then give up
				if(n.x * p0.x + n.y * p0.y + n.z * p0.z > radius) continue;

				//check if the nearest point is one of the triangle's vertices
				PxVec3 closestPoint; // closest point

				float det1p0p1, det2p0p2, det2p1p2, det0p0p1, det0p0p2, det1p1p2;
				//i = 0
				det1p0p1 = p0.dot(-(p1 - p0)); 
				det2p0p2 = p0.dot(-(p2 - p0));
				//i = 1
				det0p0p1 = p1.dot(p1 - p0);
				det2p1p2 = p1.dot(-(p2 - p1));
				//i = 2
				det0p0p2 = p2.dot(p2 - p0);
				det1p1p2 = p2.dot(p2 - p1);

				if(det1p0p1 <= 0 && det2p0p2 <= 0) closestPoint = p0;
				else if(det0p0p1 <= 0 && det2p1p2 <= 0) closestPoint = p1;
				else if(det0p0p2 <= 0 && det1p1p2 <= 0) closestPoint = p2;
				else 
				{
					//check if the nearest point is internal point of one of the triangle's edges
					float det0p0p1p2, det1p0p1p2, det2p0p1p2;
					det0p0p1p2 = det0p0p1 * det1p1p2 + det2p1p2 * p2.dot(p1 - p0);
					det1p0p1p2 = det1p0p1 * det0p0p2 - det2p0p2 * p2.dot(p1 - p0);
					det2p0p1p2 = det2p0p2 * det0p0p1 - det1p0p1 * p1.dot(p2 - p0);

					if(det0p0p1p2 <= 0) closestPoint = (p1 * det1p1p2 + p2 * det2p1p2) / (det1p1p2 + det2p1p2);
					else if(det1p0p1p2 <= 0) closestPoint = (p0 * det0p0p2 + p2 * det2p0p2) / (det0p0p2 + det2p0p2);
					else if(det2p0p1p2 <= 0) closestPoint = (p0 * det0p0p1 + p1 * det1p0p1) / (det0p0p1 + det1p0p1);
					//point is inside the triangle
					else closestPoint = (p0 * det0p0p1p2 + p1 * det1p0p1p2 + p2 * det2p0p1p2) / (det0p0p1p2 + det1p0p1p2 + det2p0p1p2);
				}

				float distSquared = closestPoint.x * closestPoint.x + closestPoint.y * closestPoint.y + closestPoint.z * closestPoint.z;
				if(distSquared > radius * radius)
				{
					continue;
				}

				if(distSquared < minDistSquared)
				{
					minDistSquared = distSquared;
					localNormal = n;
				}
			}
			normal = localNormal;

			return minDistSquared;
		}

		INPLACE_TEMPL_ARGS_DEF
#ifdef __CUDACC__
		__device__
#endif
		PX_INLINE uint32_t handleCollisions(const SimulationParams* params, INPLACE_STORAGE_ARGS_DEF, PxVec3& position, PxVec3& velocity, PxVec3& normal)
		{
			const PxPlane* memConvexPlanes = params->convexPlanes;
			const PxVec4*  memConvexVerts = params->convexVerts;
			const uint32_t*   memConvexPolygonsData = params->convexPolygonsData;

			// Algorithms are similar to CPU version
			const PxVec4*  memTrimeshVerts = params->trimeshVerts;
			const uint32_t*   memTrimeshIndices = params->trimeshIndices;

			float collisionRadius = params->collisionDistance + params->collisionThreshold;

			uint32_t numTriMeshes = params->trimeshes.getSize();
			for (uint32_t i = 0; i < numTriMeshes; ++i)
			{
				CollisionTriMeshData data;
				params->trimeshes.fetchElem(INPLACE_STORAGE_ARGS_VAL, data, i);

				if (!data.aabb.contains(position))		//check coarse bounds
				{
					continue;
				}

				PxVec3 localPosition = data.inversePose.transform(position);
									
				PxVec3 localNormal;
				float minDistSquared = checkTriangleCollision(memTrimeshVerts, memTrimeshIndices, data, collisionRadius, localPosition, localNormal);
				if (minDistSquared == PX_MAX_F32)
				{
					continue;
				}

				float penDepth = params->collisionDistance - PxSqrt(minDistSquared);

				if( penDepth > 0 )
				{
					localPosition += localNormal * penDepth;
					normal = data.pose.rotate(localNormal);
					position = data.pose.transform(localPosition);
					updateCollisionVelocity(data, normal, position, velocity);
				}
				return 1;
			}

			uint32_t numConvexMeshes = params->convexMeshes.getSize();
			for (uint32_t i = 0; i < numConvexMeshes; ++i)
			{
				CollisionConvexMeshData data;
				params->convexMeshes.fetchElem(INPLACE_STORAGE_ARGS_VAL, data, i);

				if (!data.aabb.contains(position))		//check coarse bounds
				{
					continue;
				}

				PxVec3 localPosition = data.inversePose.transform(position);

				float penDepth = UINT32_MAX;
				PxVec3 localNormal(0);

				bool insideConvex = true;
				bool insidePolygon = true;
				float distSquaredMin = UINT32_MAX;
				PxVec3 nearestPointMin;

				uint32_t polygonsDataOffset = data.polygonsDataOffset;
				for (uint32_t polyId = 0; polyId < data.numPolygons; polyId++) // for each polygon
				{		
					PxPlane plane;
					SIM_FETCH_PLANE(plane, ConvexPlanes, data.firstPlane + polyId);

					uint32_t vertCount = SIM_FETCH(ConvexPolygonsData, polygonsDataOffset);

					float dist = (localPosition.dot(plane.n) + plane.d);
					if (dist > 0) //outside convex
					{
						insideConvex = false;

						if (dist > collisionRadius)
						{
							insidePolygon = false;
							distSquaredMin = dist * dist;
							break;
						}

						insidePolygon = true;
						PxVec3 polygonNormal = plane.n;

						uint32_t begVertId = SIM_FETCH(ConvexPolygonsData, polygonsDataOffset + vertCount);
						PxVec3 begVert; splitFloat4(begVert, SIM_FETCH(ConvexVerts, data.firstVertex + begVertId));
						for (uint32_t vertId = 1; vertId <= vertCount; ++vertId) //for each vertex
						{
							uint32_t endVertId = SIM_FETCH(ConvexPolygonsData, polygonsDataOffset + vertId);
							PxVec3 endVert; splitFloat4(endVert, SIM_FETCH(ConvexVerts, data.firstVertex + endVertId));

							PxVec3 segment = endVert - begVert;
							PxVec3 segmentNormal = polygonNormal.cross(segment);
							float  sign = segmentNormal.dot(localPosition - begVert);
							if (sign < 0)
							{
								insidePolygon = false;

								float distSquared;
								PxVec3 nearestPoint;
								calculatePointSegmentSquaredDist(begVert, endVert, localPosition, distSquared, nearestPoint);
								if (distSquared < distSquaredMin)
								{
									distSquaredMin = distSquared;
									nearestPointMin = nearestPoint;
								}
							}

							begVert = endVert;
						}
						if (insidePolygon)
						{
							penDepth = params->collisionDistance - dist;
							localNormal = polygonNormal;
							break;
						}
					}

					if (insideConvex)
					{
						float penDepthPlane = params->collisionDistance - dist; //dist is negative inside
						if (penDepthPlane < penDepth) //inside convex 
						{
							penDepth = penDepthPlane; 
							localNormal = plane.n;
						}
					}
					polygonsDataOffset += (vertCount + 1);
				}

				if (!insideConvex && !insidePolygon)
				{
					if (distSquaredMin > collisionRadius * collisionRadius)
					{
						continue; //no intersection, too far away
					}
					float dist = PxSqrt(distSquaredMin);

					localNormal = localPosition - nearestPointMin;
					localNormal *= (1 / dist); //normalize
						
					penDepth = params->collisionDistance - dist;
				}

				if (penDepth > 0)
				{
					localPosition += localNormal * penDepth;
					normal = data.pose.rotate(localNormal);
					position = data.pose.transform(localPosition);
					updateCollisionVelocity(data, normal, position, velocity);
				}
				return 1;
			}

			uint32_t numBoxes = params->boxes.getSize();
			for (uint32_t i = 0; i < numBoxes; ++i)
			{
				CollisionBoxData data;
				params->boxes.fetchElem(INPLACE_STORAGE_ARGS_VAL, data, i);

				if (!data.aabb.contains(position))
				{
					continue;
				}

				PxVec3 localPosition = data.inversePose.transform(position);


				PxVec3 closestPoint = PxVec3(PxClamp(localPosition.x, -data.halfSize.x, data.halfSize.x), PxClamp(localPosition.y, -data.halfSize.y, data.halfSize.y), PxClamp(localPosition.z, -data.halfSize.z, data.halfSize.z));
				PxVec3 v = localPosition - closestPoint;
				float vMagnitudeSquared = v.magnitudeSquared();
				if(vMagnitudeSquared > collisionRadius * collisionRadius) continue; //no intersection

				PxBounds3 bounds = PxBounds3(-data.halfSize, data.halfSize);
				float penDepth;

				PxVec3 localNormal(0);
				if(vMagnitudeSquared > 0)
				{
					float vMagnitude = PxSqrt(vMagnitudeSquared);
					localNormal = v * (1 / vMagnitude);

					penDepth = params->collisionDistance - vMagnitude;
				}
				else
				{
					PxVec3 penDepth3D = PxVec3(
						data.halfSize.x - PxAbs(localPosition.x),
						data.halfSize.y - PxAbs(localPosition.y),
						data.halfSize.z - PxAbs(localPosition.z)
						);
					float penDepth3Dmin = penDepth3D.minElement();

					if (penDepth3Dmin == penDepth3D.x)
					{
						localNormal.x = localPosition.x < 0 ? -1.0f : 1.0f;
					}
					else if (penDepth3Dmin == penDepth3D.y)
					{
						localNormal.y = localPosition.y < 0 ? -1.0f : 1.0f;
					}
					else if (penDepth3Dmin == penDepth3D.z)
					{
						localNormal.z = localPosition.z < 0 ? -1.0f : 1.0f;
					}

					penDepth = params->collisionDistance + penDepth3Dmin;
				}

				normal = data.pose.rotate(localNormal);

				if (penDepth > 0)
				{
					localPosition += localNormal * penDepth;
					position = data.pose.transform(localPosition);

					updateCollisionVelocity(data, normal, position, velocity);
				}
				return 1;
			}

			uint32_t numCapsules = params->capsules.getSize();
			for (uint32_t i = 0; i < numCapsules; ++i)
			{
				CollisionCapsuleData data;
				params->capsules.fetchElem(INPLACE_STORAGE_ARGS_VAL, data, i);

				if (!data.aabb.contains(position))
				{
					continue;
				}

				PxVec3 localPosition = data.inversePose.transform(position);

				// Capsule is Minkowski sum of sphere with segment
				const float closestX = PxClamp(localPosition.x, -data.halfHeight, data.halfHeight);
				PxVec3 localNormal(localPosition.x - closestX, localPosition.y, localPosition.z);

				float distance = localNormal.magnitude();
				float penDepth = (data.radius - distance);
				// No intersection?
				if (-penDepth > params->collisionThreshold)
				{
					continue;
				}
				if (distance > 0) // avoid division by zero
				{
					localNormal /= distance;
				}
				normal = data.pose.rotate(localNormal);

				if (penDepth > 0)
				{
					localPosition = data.radius * localNormal;
					localPosition.x += closestX;

					position = data.pose.transform(localPosition);

					updateCollisionVelocity(data, normal, position, velocity);
				}
				return 1;
			}

			uint32_t numSpheres = params->spheres.getSize();
			for (uint32_t i = 0; i < numSpheres; ++i)
			{
				CollisionSphereData data;
				params->spheres.fetchElem(INPLACE_STORAGE_ARGS_VAL, data, i);

				if (!data.aabb.contains(position))
				{
					continue;
				}

				PxVec3 localNormal = data.inversePose.transform(position);
				float distance = localNormal.magnitude();

				float penDepth = (data.radius - distance);
				// No intersection?
				if (-penDepth > params->collisionThreshold)
				{
					continue;
				}
				localNormal /= distance;
				normal = data.pose.rotate(localNormal);

				if (penDepth > 0)
				{
					position = data.pose.transform(data.radius * localNormal);

					updateCollisionVelocity(data, normal, position, velocity);
				}
				return 1;
			}

			uint32_t numHalfSpaces = params->halfSpaces.getSize(); 
			for (uint32_t i = 0; i < numHalfSpaces; ++i)
			{
				CollisionHalfSpaceData data;
				params->halfSpaces.fetchElem(INPLACE_STORAGE_ARGS_VAL, data, i);

				float penDepth = (data.origin - position).dot(data.normal);

				// No intersection?
				if (-penDepth > params->collisionThreshold)
				{
					continue;
				}

				normal = data.normal;
				if (penDepth > 0)
				{
					position += penDepth * data.normal;

					updateCollisionVelocity(data, normal, position, velocity);
				}
				return 1;
			}

			return 0;
		}



#ifdef __CUDACC__
		__device__
#endif
		PX_INLINE float calcParticleBenefit(
			const InjectorParams& inj, const PxVec3& eyePos,
			const PxVec3& pos, const PxVec3& vel, float life)
		{
			float benefit = inj.mLODBias;
			//distance term
			float distance = (eyePos - pos).magnitude();
			benefit += inj.mLODDistanceWeight * (1.0f - PxMin(1.0f, distance / inj.mLODMaxDistance));
			//velocity term, TODO: clamp velocity
			float velMag = vel.magnitude();
			benefit += inj.mLODSpeedWeight * velMag;
			//life term
			benefit += inj.mLODLifeWeight * life;

			return PxClamp(benefit, 0.0f, 1.0f);
		}

		INPLACE_TEMPL_VA_ARGS_DEF(typename FieldAccessor)
#ifdef __CUDACC__
		__device__
#endif
		PX_INLINE float simulateParticle(
			const SimulationParams* params, INPLACE_STORAGE_ARGS_DEF, SIM_INJECTOR_ARRAY injectorArray,
			float deltaTime, PxVec3 gravity, PxVec3 eyePos,
			bool isNewParticle, unsigned int srcIdx, unsigned int dstIdx,
			SIM_FLOAT4* memPositionMass, SIM_FLOAT4* memVelocityLife, IofxActorIDIntl* memIofxActorIDs,
			float* memLifeSpan, float* memLifeTime, unsigned int* memInjector, SIM_FLOAT4* memCollisionNormalFlags, uint32_t* memUserData,
			FieldAccessor& fieldAccessor, unsigned int &injIndex
			)
		{
			//read
			PxVec3 position;
			PxVec3 velocity;
			float mass = splitFloat4(position, SIM_FETCH(PositionMass, srcIdx));
			splitFloat4(velocity, SIM_FETCH(VelocityLife, srcIdx));
			float lifeSpan = SIM_FETCH(LifeSpan, srcIdx);
			unsigned int injector = SIM_FETCH(Injector, srcIdx);
			IofxActorIDIntl iofxActorID = IofxActorIDIntl(SIM_FETCH(IofxActorIDs, srcIdx));

			PxVec3 collisionNormal(0.0f);
			uint32_t collisionFlags = 0;

			float lifeTime = lifeSpan;
			if (!isNewParticle)
			{
				using namespace nvidia::apex;

				lifeTime = SIM_FETCH(LifeTime, srcIdx);

				//collide using the old state
				collisionFlags = handleCollisions INPLACE_TEMPL_ARGS_VAL (params, INPLACE_STORAGE_ARGS_VAL, position, velocity, collisionNormal);

				//advance to a new state
				PxVec3 velocityDelta = deltaTime * gravity;
				fieldAccessor(srcIdx, velocityDelta);

				velocity += velocityDelta;
				position += deltaTime * velocity;

				lifeTime = PxMax(lifeTime - deltaTime, 0.0f);
			}

			InjectorParams injParams;
			SIM_FETCH_INJECTOR(injectorArray, injParams, injector);
			injIndex = injParams.mLocalIndex;
			// injParams.mLODBias == FLT_MAX if injector was released!
			// and IOFX returns IofxActorIDIntl::NO_VOLUME for homeless/dead particles
			bool validActorID = (injParams.mLODBias < FLT_MAX)
				&& (isNewParticle || (iofxActorID.getVolumeID() != IofxActorIDIntl::NO_VOLUME))
				&& position.isFinite() && velocity.isFinite();
			if (!validActorID)
			{
				iofxActorID.setActorClassID(IofxActorIDIntl::IPX_ACTOR);
				injIndex = UINT32_MAX;
			}

			//write
			memLifeTime[dstIdx] = lifeTime;

			if (!isNewParticle || dstIdx != srcIdx)
			{
				memPositionMass[dstIdx] = combineFloat4(position, mass);
				memVelocityLife[dstIdx] = combineFloat4(velocity, lifeTime / lifeSpan);
			}
			if (!validActorID || dstIdx != srcIdx)
			{
				memIofxActorIDs[dstIdx] = iofxActorID;
			}
			if (dstIdx != srcIdx)
			{
				memLifeSpan[dstIdx] = lifeSpan;
				memInjector[dstIdx] = injector;

				memUserData[dstIdx] = SIM_FETCH(UserData, srcIdx);
			}
			memCollisionNormalFlags[dstIdx] = combineFloat4(collisionNormal, SIM_INT_AS_FLOAT(collisionFlags));

			float benefit = -FLT_MAX;
			if (validActorID && lifeTime > 0.0f)
			{
				benefit = calcParticleBenefit(injParams, eyePos, position, velocity, lifeTime / lifeSpan);
			}
			return benefit;
		}
	}

} // namespace nvidia

#endif
