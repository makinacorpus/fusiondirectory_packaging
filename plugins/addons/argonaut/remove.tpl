<div style="font-size:18px;">
 <img alt="" src="images/warning.png" align=top>&nbsp;{t}Warning{/t}
</div>
<p>
  {$info}
</p>

<p>
 {t}So - if you're sure - press 'Delete' to continue or 'Cancel' to abort.{/t}
</p>

<p class="plugbottom">
{if $multiple}
  <input type=submit name="delete_multiple_confirm" value="{msgPool type=delButton}">
{else}
  <input type=submit name="delete_confirm" value="{msgPool type=delButton}">
{/if}
  <input type=submit name="delete_cancel" value="{msgPool type=cancelButton}">
</p>

