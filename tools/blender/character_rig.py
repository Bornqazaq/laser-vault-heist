"""Original T-pose rig and five authored actions; no imported motion."""
import bpy,math
from mathutils import Vector,Matrix
def create_rig():
 arm=bpy.data.armatures.new('ThiefSkeleton');rig=bpy.data.objects.new('ThiefRig',arm);bpy.context.scene.collection.objects.link(rig)
 bpy.context.view_layer.objects.active=rig;rig.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
 definitions=[('Root',(0,0,0),(0,0,.15),None),('Pelvis',(0,0,.92),(0,0,1.03),'Root'),('Spine',(0,0,1.03),(0,0,1.23),'Pelvis'),('Chest',(0,0,1.23),(0,0,1.44),'Spine'),('Neck',(0,0,1.44),(0,0,1.55),'Chest'),('Head',(0,0,1.55),(0,0,1.79),'Neck')]
 for side,s in [('L',1),('R',-1)]:
  definitions += [('Shoulder.'+side,(s*.07,0,1.42),(s*.24,0,1.42),'Chest'),('UpperArm.'+side,(s*.24,0,1.42),(s*.52,0,1.42),'Shoulder.'+side),('Forearm.'+side,(s*.52,0,1.42),(s*.77,0,1.42),'UpperArm.'+side),('Hand.'+side,(s*.77,0,1.42),(s*.91,0,1.42),'Forearm.'+side),('Thigh.'+side,(s*.12,0,.92),(s*.12,0,.48),'Pelvis'),('Shin.'+side,(s*.12,0,.48),(s*.12,0,.13),'Thigh.'+side),('Foot.'+side,(s*.12,0,.13),(s*.12,-.18,.06),'Shin.'+side)]
 for name,head,tail,parent in definitions:
  bone=arm.edit_bones.new(name);bone.head=head;bone.tail=tail
  if parent:bone.parent=arm.edit_bones[parent]
 bpy.ops.object.mode_set(mode='OBJECT');rig.show_in_front=True
 return rig
def rotation(rig,name,matrix):
 bone=rig.pose.bones[name];basis=rig.data.bones[name].matrix_local.to_3x3()
 bone.rotation_mode='QUATERNION';bone.rotation_quaternion=(basis.inverted()@matrix.to_3x3()@basis).to_quaternion()
def pose(rig,state,phase):
 for p in rig.pose.bones:p.location=(0,0,0);p.rotation_mode='QUATERNION';p.rotation_quaternion=(1,0,0,0);p.scale=(1,1,1)
 crouch=state in ('CrouchIdle','CrouchWalk');walk=state in ('Walk','CrouchWalk');breath=.005*math.sin(phase*math.tau)
 root=rig.pose.bones['Root'];root.location=rig.data.bones['Root'].matrix_local.to_3x3().inverted()@Vector((0,.08 if crouch else 0,-.57+breath if crouch else abs(math.sin(phase*math.tau))*.018 if walk else breath))
 rotation(rig,'Spine',Matrix.Rotation(math.radians(26 if crouch else 1),4,'X'));rotation(rig,'Chest',Matrix.Rotation(math.radians(18 if crouch else -1),4,'X'));rotation(rig,'Neck',Matrix.Rotation(math.radians(-14 if crouch else 0),4,'X'))
 for side,s in [('L',1),('R',-1)]:
  cycle=phase*math.tau+(0 if s==1 else math.pi);swing=math.sin(cycle)
  thigh=math.radians(-68 if crouch else 0)+(swing*(.16 if crouch else .48) if walk else 0);knee=math.radians(136 if crouch else 3)+(max(0,-swing)*(.22 if crouch else .80) if walk else 0)
  rotation(rig,'Thigh.'+side,Matrix.Rotation(thigh,4,'X'));rotation(rig,'Shin.'+side,Matrix.Rotation(knee,4,'X'));rotation(rig,'Foot.'+side,Matrix.Rotation(-(thigh+knee),4,'X'))
  swing_arm=(-swing*.25 if walk else .02*math.sin(cycle))
  upper=Matrix.Rotation(swing_arm+(.35 if crouch else 0),4,'X')@Matrix.Rotation(s*math.radians(70 if crouch else 78),4,'Y')
  rotation(rig,'UpperArm.'+side,upper);rotation(rig,'Forearm.'+side,Matrix.Rotation(-s*math.radians(20 if crouch else 8),4,'Y'))
 if state=='Interact':
  amount=math.sin(math.pi*phase)
  rotation(rig,'UpperArm.R',Matrix.Rotation(-amount*.9,4,'Z')@Matrix.Rotation(-math.radians(78)+amount*.55,4,'Y'));rotation(rig,'Forearm.R',Matrix.Rotation(amount*.55,4,'Z'))
def animate(rig):
 clips=[('Idle',48),('Walk',24),('CrouchIdle',48),('CrouchWalk',32),('Interact',24)];rig.animation_data_create()
 for name,length in clips:
  action=bpy.data.actions.new(name);action.use_fake_user=True;rig.animation_data.action=action
  for frame in range(0,length+1,2):
   pose(rig,name,frame/length)
   for bone in rig.pose.bones:
    bone.keyframe_insert(data_path='rotation_quaternion',frame=frame,group=bone.name);bone.keyframe_insert(data_path='location',frame=frame,group=bone.name)
  action['duration_frames']=length
 rig.animation_data.action=None;pose(rig,'Idle',0)
 return clips

