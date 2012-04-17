(function ($) {
  $(document).bind('afterQuicksandAnimation', function(event)
  {
    event.callback = function() {
      var time = new Date();
      console.log(time.getHours() + '-' + time.getMinutes() + '-' +
        time.getSeconds() + '-' + time.getMilliseconds() + ': after');
    };
  });

  $(document).bind('beforeQuicksandAnimation', function(event)
  {
    event.callback = function() {
      var time = new Date();
      console.log(time.getHours() + '-' + time.getMinutes() + '-' +
        time.getSeconds() + '-' + time.getMilliseconds() + ': before');
    };
  });
})(jQuery);
