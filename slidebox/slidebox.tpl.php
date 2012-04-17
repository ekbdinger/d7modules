<div id="slidebox">
  <a class="close"></a>
  <p>Next post</p>
  <h2><?php echo $title; ?></h2>
  <?php echo l("Read More &raquo;", "node/{$nid}", array('html' => TRUE, 'attributes' => array('class' => array('more')))); ?>
</div>
<div id="slidebox_manual">
  <a class="open"></a>
</div>
<div id="slidebox_cookie">
  <a class="set">Don't show recommendations again this session.</a>
</div>

