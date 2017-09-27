--create or replace type array_int as array(256) of integer;

DECLARE
   i ordsys.ordimage;
  ctx RAW(400) := NULL;
  ligne multimedia%ROWTYPE;
  cursor mm is
  select * from multimedia
  for update;
  sig1 ordsys.ordimageSignature;
  sig2 ordsys.ordimageSignature;
  sim integer;
  dist float;
  
  --type array_int is array(256) of integer;
  histoR array_int;
  histoG array_int;
  histoB array_int;
  
  tauxR float;
  tauxG float;
  tauxB float;
  
  mIsColor integer;
  mNb_pixels_contour integer;
  mMoy_norme_grad float;
  
BEGIN
   FOR compteur IN 1 .. 500 LOOP
    
    insert into multimedia(nom, image, signature)
    values (compteur||'.jpg', ordsys.ordimage.init(), ordsys.ordimageSignature.init());
    commit;
   END LOOP;
   
  FOR compteur IN 1 .. 500 LOOP
       -- chargement du contenu de l'image a partir du fichier
      select image into i
      from multimedia
      where nom = compteur||'.jpg'
      for update;
      i.importFrom(ctx, 'file', 'IMG', compteur||'.jpg');
      update multimedia
      set image = i
      where nom = compteur||'.jpg';
      commit;
   END LOOP;
   
   
 
  -- generation des signatures
  for ligne in mm loop
    ligne.signature.generateSignature(ligne.image);
    update multimedia
    set signature = ligne.signature
    where current of mm;
  end loop;
 
  commit;

END;
