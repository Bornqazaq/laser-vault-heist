"""Hand-authored stylized facial planes and a real open hood."""
import math
from common import *
from character_mesh import weighted,constant,tube
def build_face(skin,lip,brow,eye,iris,cloth,lining):
 verts=[];uvs=[];faces=[];rings=31;segments=48
 for j in range(rings):
  t=j/(rings-1);z=1.515+t*.29
  # Narrow chin, clear cheek plane and rounded cranium.
  profile=[(1.515,.044,.053),(1.55,.077,.075),(1.60,.100,.090),(1.65,.113,.097),(1.70,.112,.103),(1.75,.105,.107),(1.79,.083,.089),(1.805,.035,.045)]
  for pi in range(len(profile)-1):
   za,xa,ya=profile[pi];zb,xb,yb=profile[pi+1]
   if za<=z<=zb+1e-6:
    q=(z-za)/(zb-za);rx=xa+(xb-xa)*q;ry=ya+(yb-ya)*q;break
  for k in range(segments):
   a=math.tau*k/segments;x=rx*math.cos(a);y=ry*math.sin(a)
   front=max(0,-math.sin(a))**10
   nose=.038*math.exp(-(x/.022)**2-((z-1.665)/.043)**2)
   mouth=.008*math.exp(-(x/.038)**4-((z-1.608)/.012)**2)
   cheeks=.008*math.exp(-((abs(x)-.065)/.03)**2-((z-1.658)/.035)**2)
   sockets=.008*math.exp(-((abs(x)-.043)/.025)**2-((z-1.703)/.015)**2)
   y-=front*(nose+mouth+cheeks-sockets)
   verts.append((x,y,z));uvs.append((k/segments,t))
 for j in range(rings-1):
  for k in range(segments):
   q=(k+1)%segments;faces.append((j*segments+k,j*segments+q,(j+1)*segments+q,(j+1)*segments+k))
 faces.append(tuple(reversed(range(segments))));faces.append(tuple((rings-1)*segments+k for k in range(segments)))
 weighted(face_mesh('Face',verts,faces,skin,uvs),constant('Head'))
 nose=[(0,-.113,1.715),(-.012,-.105,1.700),(.012,-.105,1.700),(0,-.138,1.681),(0,-.153,1.657),(-.018,-.127,1.652),(.018,-.127,1.652),(0,-.140,1.646)]
 ob=weighted(face_mesh('NosePlanes',nose,[(0,1,5,4,3),(0,3,4,6,2),(5,7,4),(4,7,6)],skin),constant('Head'))
 for poly in ob.data.polygons:poly.use_smooth=False
 for side in [-1,1]:
  x=side*.044
  weighted(sphere('Eye',(x,-.099,1.703),(.018,.008,.006),eye,24,12),constant('Head'))
  weighted(sphere('Iris',(x,-.1087,1.703),(.0065,.002,.0065),iris,20,10),constant('Head'))
  weighted(sphere('Pupil',(x,-.1104,1.703),(.003,.001,.004),brow,16,8),constant('Head'))
  for upper in [True,False]:
   points=[]
   for i in range(13):
    t=i/12;points.append((x+(t-.5)*.041,-.106- .003*math.sin(t*math.pi),1.703+(1 if upper else -1)*.008*math.sin(t*math.pi)))
   tube('Eyelid',points,[.0019]*len(points),skin,constant('Head'),8)
  points=[(x-side*.024,-.101,1.723),(x,-.108,1.728),(x+side*.026,-.096,1.726)]
  tube('Eyebrow',points,[.003,.004,.002],brow,constant('Head'),8)
  weighted(sphere('Nostril',(side*.012,-.139,1.651),(.005,.002,.0025),lip,16,8),constant('Head'))
  weighted(sphere('Ear',(side*.112,.007,1.668),(.017,.018,.038),skin,20,14),constant('Head'))
 points=[(-.030,-.096,1.608),(-.014,-.103,1.611),(0,-.106,1.608),(.014,-.103,1.611),(.030,-.096,1.608)]
 tube('MouthLine',points,[.0017]*5,lip,constant('Head'),10)
 tube('LowerLip',[(-.026,-.097,1.604),(0,-.105,1.601),(.026,-.097,1.604)],[.002,.004,.002],skin,constant('Head'),10)
 # Hood shell is open at the face; the lip forms a separate inner lining.
 verts=[];uvs=[];faces=[];n=56
 for j in range(9):
  t=j/8;y=-.146+t*.285;rx=.148*(1-.27*t);rz=.185*(1-.18*t)
  for k in range(n):
   a=math.tau*k/n;x=rx*math.sin(a);z=1.674+rz*math.cos(a)
   z+=.025*max(0,math.cos(a))**12*(1-t)
   yy=y-.030*max(0,math.cos(a))**6*(1-t)
   verts.append((x,yy,z));uvs.append((k/n,t))
 for j in range(8):
  for k in range(n):
   q=(k+1)%n;faces.append((j*n+k,(j+1)*n+k,(j+1)*n+q,j*n+q))
 faces.append(tuple(8*n+k for k in range(n)))
 outer_count=len(verts)
 outer_faces=[tuple(reversed(f)) for f in faces]
 inner=[(x*.96,y+.004,1.674+(z-1.674)*.96) for x,y,z in verts]
 combined=outer_faces+[tuple(i+outer_count for i in f) for f in faces]
 for k in range(n):
  q=(k+1)%n;combined.append((k,q,q+outer_count,k+outer_count))
 weighted(face_mesh('HoodShell',verts+inner,combined,cloth,uvs+uvs),constant('Head'))
 for row in range(2):
  points=[]
  for k in range(n+1):
   a=math.tau*k/n;points.append((.148*math.sin(a),-.146-.030*max(0,math.cos(a))**6+row*.008,1.674+.185*math.cos(a)+.025*max(0,math.cos(a))**12))
  tube('HoodBinding',points,[.007]*len(points),lining,constant('Head'),8)
