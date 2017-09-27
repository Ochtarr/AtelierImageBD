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
BEGIN
   FOR compteur IN 1 .. 500 LOOP
 
    -- insertion d'une image, de contenu vide
    insert into multimedia(nom, image, signature)
    values (compteur||'.jpg', ordsys.ordimage.init(), ordsys.ordimageSignature.init());
    commit;
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